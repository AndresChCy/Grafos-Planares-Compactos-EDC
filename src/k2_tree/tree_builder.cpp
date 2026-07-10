#include "tree_builder.hpp"

#include "kTree.h"

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

extern unsigned int* positionInTH;

namespace {

bool write_voc_and_cil(TREP* trep, const fs::path& base_path)
{
    const std::string base = base_path.string();

    std::FILE* voc = std::fopen((base + ".voc").c_str(), "wb");
    if (!voc) {
        return false;
    }

    if (std::fwrite(&trep->part, sizeof(uint), 1, voc) != 1 ||
        std::fwrite(&trep->tamSubm, sizeof(uint), 1, voc) != 1 ||
        std::fwrite(&trep->numberOfNodes, sizeof(uint), 1, voc) != 1 ||
        std::fwrite(&trep->numberOfEdges, sizeof(ulong), 1, voc) != 1 ||
        std::fwrite(&trep->repK1, sizeof(uint), 1, voc) != 1 ||
        std::fwrite(&trep->repK2, sizeof(uint), 1, voc) != 1 ||
        std::fwrite(&trep->maxRealLevel1, sizeof(uint), 1, voc) != 1 ||
        std::fwrite(&trep->maxLevel1, sizeof(uint), 1, voc) != 1 ||
        std::fwrite(&trep->maxLevel2, sizeof(uint), 1, voc) != 1 ||
        std::fwrite(&trep->zeroNode, sizeof(uint), 1, voc) != 1 ||
        std::fwrite(&trep->lenWords, sizeof(uint), 1, voc) != 1) {
        std::fclose(voc);
        return false;
    }

    for (uint i = 0; i < trep->zeroNode; ++i) {
        if (std::fwrite(hash[positionInTH[i]].word, sizeof(char), trep->lenWords, voc) != trep->lenWords) {
            std::fclose(voc);
            return false;
        }
    }

    std::fclose(voc);

    std::FILE* cil = std::fopen((base + ".cil").c_str(), "wb");
    if (!cil) {
        return false;
    }

    for (uint fila = 0; fila < trep->part; ++fila) {
        for (uint columna = 0; columna < trep->part; ++columna) {
            MREP* rep = trep->submatrices[fila][columna];
            if (std::fwrite(&rep->numberOfNodes, sizeof(uint), 1, cil) != 1 ||
                std::fwrite(&rep->numberOfEdges, sizeof(ulong), 1, cil) != 1) {
                std::fclose(cil);
                return false;
            }

            if (rep->numberOfEdges == 0) {
                continue;
            }

            if (std::fwrite(&rep->cutBt, sizeof(uint), 1, cil) != 1 ||
                std::fwrite(&rep->lastBt1_len, sizeof(uint), 1, cil) != 1 ||
                std::fwrite(&rep->nleaves, sizeof(uint), 1, cil) != 1) {
                std::fclose(cil);
                return false;
            }

            saveFT(rep->compressIL, cil);
        }
    }

    std::fclose(cil);
    return true;
}

std::vector<char> to_c_buffer(const fs::path& path)
{
    std::string text = path.string();
    std::vector<char> buffer(text.begin(), text.end());
    buffer.push_back('\0');
    return buffer;
}


std::uint64_t file_size_or_zero(const fs::path& path)
{
    std::error_code ec;
    if (!fs::exists(path, ec) || ec) {
        return 0ULL;
    }

    const auto size = fs::file_size(path, ec);
    if (ec) {
        return 0ULL;
    }

    return static_cast<std::uint64_t>(size);
}

} // namespace

K2TreePreset K2TreeBuilder::small()
{
    return K2TreePreset{"small", 4, 2, 5, 18, 4'000'000ULL};
}

K2TreePreset K2TreeBuilder::large()
{
    return K2TreePreset{"large", 4, 2, 6, 20, 4'000'000ULL};
}

K2TreeBuilder::K2TreeBuilder(fs::path graph_path, K2TreePreset preset, fs::path output_root)
    : graph_path_(std::move(graph_path)),
      preset_(std::move(preset)),
      output_root_(std::move(output_root))
{
    const std::string stem = graph_path_.stem().string();
    std::ostringstream oss;
    oss << stem << '_' << preset_.name
        << "_k1_" << preset_.k1
        << "_k2_" << preset_.k2
        << "_ml1_" << preset_.max_level1
        << "_s_" << preset_.s;

    artifact_base_ = output_root_ / oss.str();
    fs::create_directories(output_root_);
}

K2TreeBuilder::~K2TreeBuilder()
{
    if (tree_) {
        destroyTreeRepresentation(tree_);
        tree_ = nullptr;
    }
}

std::vector<K2TreeBuilder::Edge> K2TreeBuilder::load_edges(const fs::path& graph_path, std::uint32_t& node_count)
{
    std::ifstream input(graph_path);
    if (!input.is_open()) {
        return {};
    }

    std::uint32_t edge_count = 0;
    if (!(input >> node_count >> edge_count)) {
        return {};
    }

    std::vector<Edge> edges;
    edges.reserve(edge_count);

    for (std::uint32_t i = 0; i < edge_count; ++i) {
        std::uint32_t source = 0;
        std::uint32_t target = 0;
        if (!(input >> source >> target)) {
            return {};
        }
        edges.push_back(Edge{source, target});
    }

    return edges;
}

void K2TreeBuilder::sort_edges(std::vector<Edge>& edges)
{
    std::sort(edges.begin(), edges.end(), [](const Edge& left, const Edge& right) {
        if (left.source != right.source) {
            return left.source < right.source;
        }
        return left.target < right.target;
    });
}

bool K2TreeBuilder::build()
{
    std::cerr << "build: start\n";
    if (tree_) {
        destroyTreeRepresentation(tree_);
        tree_ = nullptr;
    }

    std::cerr << "build: load_edges\n";
    std::uint32_t node_count = 0;
    auto edges = load_edges(graph_path_, node_count);
    if (node_count == 0) {
        std::cerr << "build: no nodes\n";
        return false;
    }

    sort_edges(edges);

    node_count_ = node_count;
    edge_count_ = static_cast<std::uint64_t>(edges.size());

    const int max_real_level1 = std::max(0, static_cast<int>(std::ceil(std::log(static_cast<double>(node_count)) /
                                                                         std::log(static_cast<double>(preset_.k1)))) - 1);
    const int max_level2 = std::max(0, static_cast<int>(std::ceil(std::log(static_cast<double>(node_count)) /
                                                                  std::log(static_cast<double>(preset_.k2)))) - 1);
    const unsigned int stored_max_level2 = static_cast<unsigned int>(max_level2 + 2);

    if (preset_.max_level1 > max_real_level1) {
        std::cerr << "build: preset max_level1 invalid for graph\n";
        return false;
    }

    std::cerr << "build: createKTree\n";
    NODE* root = createKTree(preset_.k1, preset_.k2, max_real_level1, preset_.max_level1, static_cast<int>(stored_max_level2));
    if (!root) {
        std::cerr << "build: createKTree failed\n";
        return false;
    }

    std::cerr << "build: insertNode\n";

    for (const auto& edge : edges) {
        insertNode(root, static_cast<int>(edge.source), static_cast<int>(edge.target));
    }

    std::cerr << "build: createRepresentation\n";
    MREP* rep = createRepresentation(root, node_count, static_cast<unsigned long>(edges.size()));
    if (!rep) {
        std::cerr << "build: createRepresentation failed\n";
        return false;
    }

    std::cerr << "build: createTreeRep\n";
    TREP* trep = createTreeRep(
        node_count,
        static_cast<unsigned long>(edges.size()),
        1U,
        1U << preset_.s,
        static_cast<unsigned int>(max_real_level1),
        static_cast<unsigned int>(preset_.max_level1),
        stored_max_level2,
        static_cast<unsigned int>(preset_.k1),
        static_cast<unsigned int>(preset_.k2));

    if (!trep) {
        destroyRepresentation(rep);
        std::cerr << "build: createTreeRep failed\n";
        return false;
    }

    insertIntoTreeRep(trep, rep, 0U, 0U);

    auto base_buffer = to_c_buffer(artifact_base_);
    std::cerr << "build: saveBeforeCompressInformationLeaves\n";
    saveBeforeCompressInformationLeaves(trep, base_buffer.data());
    std::cerr << "build: compressInformationLeaves\n";
    compressInformationLeaves(trep);

    std::cerr << "build: write_voc_and_cil\n";
    const bool voc_cil_ok = write_voc_and_cil(trep, artifact_base_);

    // compressInformationLeaves() (above) built a transient hash table +
    // MemoryManager + positionInTH buffer (vocabulary dedup/frequency
    // counting for the leaves) that write_voc_and_cil() just finished
    // reading from (hash[positionInTH[i]].word). Nothing after this point
    // needs them, and nothing else ever freed them: freeHashTable() existed
    // but was never called anywhere, and it didn't free positionInTH
    // either. Every previous call to build() permanently leaked the hash
    // table, its MemoryManager (word storage blocks), and positionInTH.
    freeHashTable();
    free(positionInTH);
    positionInTH = nullptr;

    if (!voc_cil_ok) {
        std::cerr << "build: write_voc_and_cil failed\n";
        destroyTreeRepAfterCompress(trep);
        return false;
    }

    // `trep` ya quedo completamente volcado a disco (.tr/.lv/.il/.voc/.cil).
    // A partir de aqui la version "de verdad" es la que se relee desde disco
    // en `tree_` (loadTreeRepresentation, mas abajo), asi que liberamos este
    // TREP intermedio en vez de dejarlo filtrado (leak) durante toda la vida
    // del K2TreeBuilder. Antes esto duplicaba en memoria toda la estructura
    // comprimida (bitmaps bt/bn + hojas) durante cada build().
    //
    // Importante: usamos destroyTreeRepAfterCompress() y NO
    // destroyTreeRepresentation(). saveBeforeCompressInformationLeaves() (mas
    // arriba) ya destruyo bt/bn de cada submatriz y ya libero los buffers de
    // cola propios de trep como parte de su propio manejo de memoria a mitad
    // de pipeline; volver a llamar destroyTreeRepresentation() aqui
    // liberaria esos mismos punteros por segunda vez (double free /
    // use-after-free).
    destroyTreeRepAfterCompress(trep);
    trep = nullptr;

    if (tree_) {
        destroyTreeRepresentation(tree_);
        tree_ = nullptr;
    }
    std::cerr << "build: loadTreeRepresentation\n";
    tree_ = loadTreeRepresentation(base_buffer.data());
    std::cerr << "build: done\n";
    return tree_ != nullptr;
}

bool K2TreeBuilder::saveCompressed()
{
    return tree_ != nullptr;
}

bool K2TreeBuilder::reloadCompressed()
{
    if (tree_) {
        destroyTreeRepresentation(tree_);
        tree_ = nullptr;
    }

    auto base_buffer = to_c_buffer(artifact_base_);
    tree_ = loadTreeRepresentation(base_buffer.data());
    return tree_ != nullptr;
}

bool K2TreeBuilder::contains(std::uint32_t source, std::uint32_t target) const
{
    if (!tree_) {
        return false;
    }

    return compactTreeCheckLink(tree_, source, target) != 0U;
}

std::vector<std::uint32_t> K2TreeBuilder::adjacency(std::uint32_t source) const
{
    std::vector<std::uint32_t> result;
    if (!tree_) {
        return result;
    }

    unsigned int* raw = compactTreeAdjacencyList(tree_, static_cast<int>(source));
    if (!raw) {
        return result;
    }

    const std::uint32_t count = raw[0];
    result.reserve(count);
    for (std::uint32_t i = 1; i <= count; ++i) {
        result.push_back(raw[i]);
    }

    return result;
}

std::vector<std::uint32_t> K2TreeBuilder::inverse(std::uint32_t target) const
{
    std::vector<std::uint32_t> result;
    if (!tree_) {
        return result;
    }

    unsigned int* raw = compactTreeInverseList(tree_, static_cast<int>(target));
    if (!raw) {
        return result;
    }

    const std::uint32_t count = raw[0];
    result.reserve(count);
    for (std::uint32_t i = 1; i <= count; ++i) {
        result.push_back(raw[i]);
    }

    return result;
}

std::size_t K2TreeBuilder::degree(std::uint32_t vertex) const
{
    if (!tree_) {
        return 0;
    }

    unsigned int* raw = compactTreeAdjacencyList(tree_, vertex);

    if (!raw) {
        return 0;
    }

    return raw[0];
}

bool K2TreeBuilder::neighbors(std::uint32_t u, std::uint32_t v) const
{
    return contains(u, v);
}

K2TreeBuilder::CompressionSizes K2TreeBuilder::compressionSizes() const
{
    CompressionSizes sizes;

    // Tamaño "antes de comprimir": la matriz de adyacencia completa representada
    // como un bitmap denso de n x n bits. Es el punto de comparacion estandar
    // para estructuras compactas como el k2-tree y la representacion de Turan.
    const std::uint64_t n = static_cast<std::uint64_t>(node_count_);
    sizes.uncompressed_bytes = (n * n + 7ULL) / 8ULL;

    const std::string base = artifact_base_.string();
    sizes.tr_bytes  = file_size_or_zero(base + ".tr");
    sizes.lv_bytes  = file_size_or_zero(base + ".lv");
    sizes.il_bytes  = file_size_or_zero(base + ".il");
    sizes.voc_bytes = file_size_or_zero(base + ".voc");
    sizes.cil_bytes = file_size_or_zero(base + ".cil");

    // Representacion final que realmente se vuelve a leer desde disco
    // (ver loadTreeRepresentation en kTree.c): tr + lv + voc + cil.
    // El .il es un archivo intermedio, superado tras compressInformationLeaves(),
    // por lo que no se cuenta en el tamaño comprimido final.
    sizes.compressed_bytes = sizes.tr_bytes + sizes.lv_bytes + sizes.voc_bytes + sizes.cil_bytes;

    if (sizes.uncompressed_bytes > 0) {
        sizes.compression_ratio =
            static_cast<double>(sizes.compressed_bytes) / static_cast<double>(sizes.uncompressed_bytes);
    }

    return sizes;
}
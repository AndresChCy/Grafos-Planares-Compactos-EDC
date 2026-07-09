#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct treeRep;
using TREP = treeRep;

extern "C" {
struct node;
struct matrixRep;

using NODE = node;
using MREP = matrixRep;

NODE* createKTree(int _K1, int _K2, int max_real_level1, int maxlevel1, int maxlevel2);
void insertNode(NODE* root, int x, int y);
MREP* createRepresentation(NODE* root, unsigned int numberOfNodes, unsigned long numberOfEdges);
MREP* loadRepresentation(char* basename);
void destroyRepresentation(MREP* rep);

TREP* createTreeRep(unsigned int nodesOrig, unsigned long edges, unsigned int part, unsigned int subm,
					unsigned int max_real_level1, unsigned int max_level1, unsigned int max_level2,
					unsigned int _K1, unsigned int _K2);
void insertIntoTreeRep(TREP* trep, MREP* rep, unsigned int i, unsigned int j);
void saveBeforeCompressInformationLeaves(TREP* trep, char* basename);
void compressInformationLeaves(TREP* trep);
void closePartialFile(void);
void partialdestroyTreeRepresentation(TREP* trep);
void saveTreeRep(TREP* trep, char* basename);
void freeHashTable(void);  // libera hash[] y _memMgr (ver hash.c); usada al final de K2TreeBuilder::build()
TREP* loadTreeRepresentation(char* basename);
unsigned int compactTreeCheckLink(TREP* trep, unsigned int x, unsigned int y);
unsigned int* compactTreeAdjacencyList(TREP* trep, int x);
unsigned int* compactTreeInverseList(TREP* trep, int y);
void destroyTreeRepresentation(TREP* trep);
void destroyTreeRepAfterCompress(TREP* trep);
}

struct K2TreePreset {
	std::string name;
	int k1 = 4;
	int k2 = 2;
	int max_level1 = 5;
	int s = 18;
	std::uint64_t hash_size = 4'000'000ULL;
};

class K2TreeBuilder {
public:
	// Tamaños en bytes antes y despues de la compresion, mas el detalle por archivo.
	struct CompressionSizes {
		std::uint64_t uncompressed_bytes = 0;  // matriz de adyacencia completa (n*n bits) sin comprimir
		std::uint64_t tr_bytes = 0;
		std::uint64_t lv_bytes = 0;
		std::uint64_t il_bytes = 0;            // archivo intermedio, no forma parte del arbol final
		std::uint64_t voc_bytes = 0;
		std::uint64_t cil_bytes = 0;
		std::uint64_t compressed_bytes = 0;    // tr + lv + voc + cil (lo que realmente se vuelve a leer)
		double compression_ratio = 0.0;        // compressed_bytes / uncompressed_bytes
	};

	static K2TreePreset small();
	static K2TreePreset large();

	K2TreeBuilder(fs::path graph_path, K2TreePreset preset, fs::path output_root = "cds/k2_trees");
	~K2TreeBuilder();

	K2TreeBuilder(const K2TreeBuilder&) = delete;
	K2TreeBuilder& operator=(const K2TreeBuilder&) = delete;

	bool build();
	bool saveCompressed();
	bool reloadCompressed();

	bool contains(std::uint32_t source, std::uint32_t target) const;
	std::vector<std::uint32_t> adjacency(std::uint32_t source) const;
	std::vector<std::uint32_t> inverse(std::uint32_t target) const;

	const fs::path& graph_path() const { return graph_path_; }
	const K2TreePreset& preset() const { return preset_; }
	const fs::path& output_root() const { return output_root_; }
	const fs::path& artifact_base() const { return artifact_base_; }

	std::size_t degree(std::uint32_t vertex) const;
	bool neighbors(std::uint32_t u, std::uint32_t v) const;

	std::uint32_t node_count() const { return node_count_; }
	std::uint64_t edge_count() const { return edge_count_; }

	// Requiere haber llamado build() (y opcionalmente saveCompressed()) antes,
	// para que los archivos .tr/.lv/.voc/.cil ya existan en disco.
	CompressionSizes compressionSizes() const;

private:
	struct Edge {
		std::uint32_t source = 0;
		std::uint32_t target = 0;
	};

	static bool convert_pg_to_binary(const fs::path& graph_path, const fs::path& binary_path);
	static std::vector<Edge> load_edges(const fs::path& graph_path, std::uint32_t& node_count);
	static void sort_edges(std::vector<Edge>& edges);

	fs::path graph_path_;
	K2TreePreset preset_;
	fs::path output_root_;
	fs::path artifact_base_;
	fs::path temp_binary_path_;
	TREP* tree_ = nullptr;
	bool built_ = false;
	bool compressed_ = false;
	std::uint32_t node_count_ = 0;
	std::uint64_t edge_count_ = 0;
};
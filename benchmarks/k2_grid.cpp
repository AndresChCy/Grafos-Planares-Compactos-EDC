#include "tree_builder.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

double elapsed_ms(const std::chrono::steady_clock::time_point& start,
                  const std::chrono::steady_clock::time_point& end)
{
    return std::chrono::duration<double, std::milli>(end - start).count();
}

std::uint64_t file_size_or_zero(const fs::path& path)
{
    std::error_code ec;
    if (!fs::exists(path, ec) || ec) {
        return 0ULL;
    }

    auto size = fs::file_size(path, ec);
    if (ec) {
        return 0ULL;
    }

    return static_cast<std::uint64_t>(size);
}

void write_csv_header(std::ofstream& csv)
{
    csv << "preset,k1,k2,max_level1,s,build_ms,save_ms,"
           "tr_bytes,lv_bytes,il_bytes,voc_bytes,cil_bytes,compressed_bytes\n";
}

void write_csv_row(std::ofstream& csv,
                   const K2TreePreset& preset,
                   const K2TreeBuilder& builder,
                   double build_ms,
                   double save_ms)
{
    const fs::path base = builder.artifact_base();
    const std::uint64_t tr_bytes = file_size_or_zero(base.string() + ".tr");
    const std::uint64_t lv_bytes = file_size_or_zero(base.string() + ".lv");
    const std::uint64_t il_bytes = file_size_or_zero(base.string() + ".il");
    const std::uint64_t voc_bytes = file_size_or_zero(base.string() + ".voc");
    const std::uint64_t cil_bytes = file_size_or_zero(base.string() + ".cil");
    const std::uint64_t compressed_bytes = tr_bytes + voc_bytes + cil_bytes;

    csv << preset.name << ","
        << preset.k1 << ","
        << preset.k2 << ","
        << preset.max_level1 << ","
        << preset.s << ","
        << build_ms << ","
        << save_ms << ","
        << tr_bytes << ","
        << lv_bytes << ","
        << il_bytes << ","
        << voc_bytes << ","
        << cil_bytes << ","
        << compressed_bytes << "\n";
}

} // namespace

int main(int argc, char* argv[])
{
    fs::path graph_path = (argc > 1) ? fs::path(argv[1]) : fs::path("input/tiger_map_hawaii.pg");
    fs::path output_root = (argc > 2) ? fs::path(argv[2]) : fs::path("cds/k2_trees");
    fs::path csv_path = (argc > 3) ? fs::path(argv[3]) : fs::path("results/k2_tree_benchmark.csv");

    fs::create_directories(output_root);
    fs::create_directories(csv_path.parent_path());

    std::ofstream csv(csv_path, std::ios::out | std::ios::trunc);
    if (!csv.is_open()) {
        std::cerr << "No se pudo crear el CSV: " << csv_path << '\n';
        return 1;
    }

    write_csv_header(csv);

    const std::vector<K2TreePreset> presets = {
        K2TreeBuilder::small(),
        K2TreeBuilder::large(),
    };

    for (const auto& preset : presets) {
        K2TreeBuilder builder(graph_path, preset, output_root);

        std::cout << "Construyendo " << preset.name << " -> "
                  << builder.artifact_base() << '\n';

        const auto build_start = std::chrono::steady_clock::now();
        if (!builder.build()) {
            std::cerr << "Fallo la construccion de " << preset.name << '\n';
            continue;
        }
        const auto build_end = std::chrono::steady_clock::now();

        const auto save_start = std::chrono::steady_clock::now();
        if (!builder.saveCompressed()) {
            std::cerr << "Fallo la compresion/guardado de " << preset.name << '\n';
            continue;
        }
        const auto save_end = std::chrono::steady_clock::now();

        if (!builder.reloadCompressed()) {
            std::cerr << "Fallo la recarga de " << preset.name << '\n';
            continue;
        }

        const auto adjacency_zero = builder.adjacency(0);
        if (!adjacency_zero.empty()) {
            const auto first_target = adjacency_zero.front();
            const bool ok = builder.contains(0, first_target);
            std::cout << "  consulta ejemplo 0->" << first_target << " = "
                      << (ok ? "ok" : "fallo") << '\n';
        }

        std::cout << "  artefactos en " << output_root << '\n';
        std::cout << "  build=" << elapsed_ms(build_start, build_end) << " ms"
                  << ", save=" << elapsed_ms(save_start, save_end) << " ms\n";

        write_csv_row(csv,
                      preset,
                      builder,
                      elapsed_ms(build_start, build_end),
                      elapsed_ms(save_start, save_end));
    }

    std::cout << "CSV generado en: " << csv_path << '\n';
    return 0;
}

#include "k2_tree/grid_search.hpp"
#include "bench-lib/benchmark.hpp"


#include <iostream>


int main(int argc, char* argv[]) {
    
    Paths paths;

    paths.graph_path   = "input/tiger_map_hawaii.pg";
    paths.build_exe    = "./build_tree";
    paths.compress_exe = "./compress_leaves";
    paths.csv_path     = "grid_search_k2_tree.csv";
    paths.hash_size    = 4000000;

    std::cout << "build_tree: " << BUILD_TREE_EXE << '\n';
    std::cout << "compress:   " << COMPRESS_EXE << '\n';

    // Grid recomendado para comenzar cerca de la documentación.
    // Ajusta estas listas cuando quieras explorar más o menos.
    const std::vector<int> k1_values = {2, 3, 4, 5, 6};
    const std::vector<int> k2_values = {2, 3, 4};
    const std::vector<int> max_level1_values = {4, 5, 6};
    const std::vector<int> s_values = {18, 20, 22};

    std::ofstream csv(paths.csv_path, std::ios::out | std::ios::trunc);
    if (!csv.is_open()) {
        std::cerr << "No se pudo crear el CSV: " << paths.csv_path << "\n";
        return 1;
    }

    write_csv_header(csv);

    std::optional<RunMetrics> best_by_time;
    std::optional<RunMetrics> best_by_space;

    for (int k1 : k1_values) {
        for (int k2 : k2_values) {
            for (int max_level1 : max_level1_values) {
                for (int s : s_values) {
                    std::cout << "Ejecutando K1=" << k1
                              << " K2=" << k2
                              << " maxLevel1=" << max_level1
                              << " S=" << s << " ... ";

                    RunMetrics m = run_one_configuration(paths, k1, k2, max_level1, s);
                    write_csv_row(csv, paths, m);

                    if (m.status == "OK") {
                        std::cout << "ok | "
                                  << "build=" << m.build_ms << " ms, "
                                  << "compress=" << m.compress_ms << " ms, "
                                  << "space=" << m.compressed_bytes << " bytes\n";

                        if (!best_by_time || m.total_ms < best_by_time->total_ms) {
                            best_by_time = m;
                        }
                        if (!best_by_space || m.compressed_bytes < best_by_space->compressed_bytes) {
                            best_by_space = m;
                        }
                    } else {
                        std::cout << "fallo (" << m.status << ")\n";
                    }
                }
            }
        }
    }

    csv.close();

    std::cout << "\n=== Mejor por tiempo total ===\n";
    if (best_by_time) {
        const auto& m = *best_by_time;
        std::cout << "K1=" << m.k1
                  << " K2=" << m.k2
                  << " maxLevel1=" << m.max_level1
                  << " S=" << m.s
                  << " | total=" << m.total_ms << " ms\n";
    } else {
        std::cout << "No hubo ejecuciones exitosas.\n";
    }

    std::cout << "\n=== Mejor por espacio comprimido ===\n";
    if (best_by_space) {
        const auto& m = *best_by_space;
        std::cout << "K1=" << m.k1
                  << " K2=" << m.k2
                  << " maxLevel1=" << m.max_level1
                  << " S=" << m.s
                  << " | compressed=" << m.compressed_bytes << " bytes\n";
    } else {
        std::cout << "No hubo ejecuciones exitosas.\n";
    }

    std::cout << "\nCSV generado en: " << paths.csv_path << "\n";
    return 0;
}
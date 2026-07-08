#ifndef GRID_SEARCH_HPP
#define GRID_SEARCH_HPP

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct RunMetrics {
    int k1 = 0;
    int k2 = 0;
    int max_level1 = 0;
    int s = 0;

    std::string basename;

    bool build_ok = false;
    bool compress_ok = false;

    int build_exit_code = -1;
    int compress_exit_code = -1;

    double build_ms = 0.0;
    double compress_ms = 0.0;
    double total_ms = 0.0;

    std::uint64_t tr_bytes = 0;
    std::uint64_t il_bytes = 0;
    std::uint64_t voc_bytes = 0;
    std::uint64_t cil_bytes = 0;
    std::uint64_t compressed_bytes = 0;

    std::string status = "OK";
};

struct Paths {
    fs::path graph_path;
    fs::path build_exe;
    fs::path compress_exe;
    fs::path csv_path;
    std::uint64_t hash_size = 4000000ULL;
};

struct TimedCommandResult {
    bool ok = false;
    int exit_code = -1;
    double ms = 0.0;
};

std::string make_basename(
    int k1,
    int k2,
    int max_level1,
    int s);

std::string shell_quote(const fs::path& p);

bool file_exists(const fs::path& p);

std::uint64_t file_size_or_zero(const fs::path& p);

void remove_if_exists(const fs::path& p);

void cleanup_artifacts(const std::string& base);

int system_exit_code(int raw_status);

TimedCommandResult run_command_timed(const std::string& cmd);

bool run_build_tree(
    const Paths& paths,
    const std::string& basename,
    int k1,
    int k2,
    int max_level1,
    int s,
    RunMetrics& m);

bool run_compress_leaves(
    const Paths& paths,
    const std::string& basename,
    std::uint64_t hash_size,
    RunMetrics& m);

void measure_sizes(
    const std::string& basename,
    RunMetrics& m);

RunMetrics run_one_configuration(
    const Paths& paths,
    int k1,
    int k2,
    int max_level1,
    int s);

void write_csv_header(std::ofstream& csv);

void write_csv_row(
    std::ofstream& csv,
    const Paths& paths,
    const RunMetrics& m);

std::optional<fs::path> first_existing(
    const std::vector<fs::path>& candidates);

Paths resolve_paths(
    int argc,
    char* argv[]);

void print_banner(
    const Paths& paths);

// Lee solo el primer uint (cantidad de nodos) del header del archivo de
// grafo, sin cargar nada mas. Devuelve 0 si no pudo abrirlo.
std::uint32_t read_node_count(const fs::path& graph_path);

// Replica la formula de build_tree.c:
//   max_real_level1 = ceil(log(nodes)/log(K1)) - 1
// (con nodes = numero TOTAL de nodos, ya que para los s tipicos del
// grid el grafo entra en una sola particion: tamSubm = 1<<s suele ser
// >= nodes, asi que la celda tiene el mismo tamano que el grafo
// completo. Si en tu caso tamSubm < nodes para algun s, esta
// aproximacion ya no es exacta para ese punto del grid; ver nota en
// el README).
int max_real_level1_for(std::uint32_t nodes, int k1);

// true si max_level1 es compatible con max_real_level1_for(nodes, k1).
bool max_level1_is_valid(std::uint32_t nodes, int k1, int max_level1);

struct SetupCheck {
    bool ok = true;
    std::string error;
};

// Verifica que el grafo y los dos ejecutables existan ANTES de arrancar
// el grid completo, para fallar rapido con un mensaje claro en vez de
// producir N filas "BUILD_FAILED" en el CSV.
SetupCheck validate_setup(const Paths& paths);

#endif
#include "grid_search.hpp"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <limits>
#include <sstream>

#ifndef _WIN32
#include <sys/wait.h>
#endif

std::string make_basename(
    int k1,
    int k2,
    int max_level1,
    int s)
{
    std::ostringstream oss;
    oss << "grid_k1_" << k1
        << "_k2_" << k2
        << "_ml1_" << max_level1
        << "_s_" << s;

    return oss.str();
}

std::string shell_quote(const fs::path& p)
{
    std::string s = p.string();

    std::string out = "'";

    for (char c : s) {
        if (c == '\'')
            out += "'\\''";
        else
            out += c;
    }

    out += "'";
    return out;
}

bool file_exists(const fs::path& p)
{
    std::error_code ec;
    return fs::exists(p, ec);
}

std::uint64_t file_size_or_zero(const fs::path& p)
{
    std::error_code ec;

    if (!fs::exists(p, ec) || ec)
        return 0ULL;

    auto sz = fs::file_size(p, ec);

    if (ec)
        return 0ULL;

    return static_cast<std::uint64_t>(sz);
}

void remove_if_exists(const fs::path& p)
{
    std::error_code ec;

    if (fs::exists(p, ec))
        fs::remove(p, ec);
}

void cleanup_artifacts(const std::string& base)
{
    remove_if_exists(base + ".tr");
    remove_if_exists(base + ".il");
    remove_if_exists(base + ".voc");
    remove_if_exists(base + ".cil");
}

int system_exit_code(int raw_status)
{
#ifdef _WIN32
    return raw_status;
#else
    if (raw_status == -1)
        return -1;

    if (WIFEXITED(raw_status))
        return WEXITSTATUS(raw_status);

    if (WIFSIGNALED(raw_status))
        return 128 + WTERMSIG(raw_status);

    return raw_status;
#endif
}

TimedCommandResult run_command_timed(const std::string& cmd)
{
    using clock = std::chrono::steady_clock;

    auto t0 = clock::now();

    int raw = std::system(cmd.c_str());

    auto t1 = clock::now();

    TimedCommandResult r;

    r.exit_code = system_exit_code(raw);
    r.ok = (r.exit_code == 0);
    r.ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    return r;
}

bool run_build_tree(
    const Paths& paths,
    const std::string& basename,
    int k1,
    int k2,
    int max_level1,
    int s,
    RunMetrics& m)
{
    std::ostringstream cmd;

    cmd << shell_quote(paths.build_exe) << " "
        << shell_quote(paths.graph_path) << " "
        << shell_quote(basename) << " "
        << k1 << " "
        << k2 << " "
        << max_level1 << " "
        << s;

    auto res = run_command_timed(cmd.str());

    m.build_ms = res.ms;
    m.build_exit_code = res.exit_code;
    m.build_ok = res.ok;

    return res.ok;
}

bool run_compress_leaves(
    const Paths& paths,
    const std::string& basename,
    std::uint64_t hash_size,
    RunMetrics& m)
{
    std::ostringstream cmd;

    cmd << shell_quote(paths.compress_exe) << " "
        << shell_quote(basename) << " "
        << hash_size;

    auto res = run_command_timed(cmd.str());

    m.compress_ms = res.ms;
    m.compress_exit_code = res.exit_code;
    m.compress_ok = res.ok;

    return res.ok;
}

void measure_sizes(
    const std::string& basename,
    RunMetrics& m)
{
    m.tr_bytes  = file_size_or_zero(basename + ".tr");
    m.il_bytes  = file_size_or_zero(basename + ".il");
    m.voc_bytes = file_size_or_zero(basename + ".voc");
    m.cil_bytes = file_size_or_zero(basename + ".cil");

    m.compressed_bytes =
        m.tr_bytes +
        m.voc_bytes +
        m.cil_bytes;
}

RunMetrics run_one_configuration(
    const Paths& paths,
    int k1,
    int k2,
    int max_level1,
    int s)
{
    RunMetrics m;

    m.k1 = k1;
    m.k2 = k2;
    m.max_level1 = max_level1;
    m.s = s;

    m.basename =
        make_basename(
            k1,
            k2,
            max_level1,
            s);

    cleanup_artifacts(m.basename);

    bool build_ok =
        run_build_tree(
            paths,
            m.basename,
            k1,
            k2,
            max_level1,
            s,
            m);

    if (!build_ok) {
        m.status = "BUILD_FAILED";
        m.total_ms = m.build_ms;

        measure_sizes(m.basename, m);

        return m;
    }

    bool compress_ok =
        run_compress_leaves(
            paths,
            m.basename,
            paths.hash_size,
            m);

    if (!compress_ok) {
        m.status = "COMPRESS_FAILED";
        m.total_ms = m.build_ms + m.compress_ms;

        measure_sizes(m.basename, m);

        return m;
    }

    m.total_ms = m.build_ms + m.compress_ms;

    measure_sizes(m.basename, m);

    return m;
}

void write_csv_header(std::ofstream& csv)
{
    csv << "graph,k1,k2,max_level1,s,hash_size,"
           "build_ms,compress_ms,total_ms,"
           "tr_bytes,il_bytes,voc_bytes,"
           "cil_bytes,compressed_bytes,status\n";
}

void write_csv_row(
    std::ofstream& csv,
    const Paths& paths,
    const RunMetrics& m)
{
    csv << paths.graph_path.string() << ","
        << m.k1 << ","
        << m.k2 << ","
        << m.max_level1 << ","
        << m.s << ","
        << paths.hash_size << ","
        << m.build_ms << ","
        << m.compress_ms << ","
        << m.total_ms << ","
        << m.tr_bytes << ","
        << m.il_bytes << ","
        << m.voc_bytes << ","
        << m.cil_bytes << ","
        << m.compressed_bytes << ","
        << m.status << "\n";
}

std::optional<fs::path> first_existing(
    const std::vector<fs::path>& candidates)
{
    for (const auto& p : candidates) {
        if (file_exists(p))
            return p;
    }

    return std::nullopt;
}

Paths resolve_paths(
    int argc,
    char* argv[])
{
    Paths p;

#ifdef _WIN32
    const char* exe_suffix = ".exe";
#else
    const char* exe_suffix = "";
#endif

    auto executable_name = [exe_suffix](const char* base) {
        return std::string(base) + exe_suffix;
    };

    if (argc > 1) p.graph_path = argv[1];
    if (argc > 2) p.build_exe = argv[2];
    if (argc > 3) p.compress_exe = argv[3];
    if (argc > 4) p.csv_path = argv[4];
    if (argc > 5) p.hash_size = std::stoull(argv[5]);

    if (p.graph_path.empty()) {
        auto g = first_existing({
            "input/tiger_map_hawaii.pg",
            "../input/tiger_map_hawaii.pg"
        });

        p.graph_path =
            g ? *g : fs::path("input/tiger_map_hawaii.pg");
    }

    if (p.build_exe.empty()) {
        auto b = first_existing({
            fs::path("build") / executable_name("build_tree"),
            fs::path(executable_name("build_tree")),
            fs::path(executable_name("./build_tree")),
            fs::current_path() / executable_name("build_tree")
        });

        p.build_exe =
            b ? *b : fs::path("build") / executable_name("build_tree");
    }

    if (p.compress_exe.empty()) {
        auto c = first_existing({
            fs::path("build") / executable_name("compress_leaves"),
            fs::path(executable_name("compress_leaves")),
            fs::path(executable_name("./compress_leaves")),
            fs::current_path() / executable_name("compress_leaves")
        });

        p.compress_exe =
            c ? *c : fs::path("build") / executable_name("compress_leaves");
    }

    if (p.csv_path.empty()) {
        p.csv_path = "grid_search_k2_tree.csv";
    }

    return p;
}

void print_banner(const Paths& paths)
{
    std::cout << "Graph:           " << paths.graph_path << "\n";
    std::cout << "build_tree exe:   " << paths.build_exe << "\n";
    std::cout << "compress_leaves:  " << paths.compress_exe << "\n";
    std::cout << "CSV output:       " << paths.csv_path << "\n";
    std::cout << "hash size:        " << paths.hash_size << "\n\n";
}

SetupCheck validate_setup(const Paths& paths)
{
    SetupCheck check;

    if (!file_exists(paths.graph_path)) {
        check.ok = false;
        check.error = "No existe el archivo de grafo: " + paths.graph_path.string();
        return check;
    }

    if (!file_exists(paths.build_exe)) {
        check.ok = false;
        check.error = "No existe el ejecutable build_tree: " + paths.build_exe.string();
        return check;
    }

    if (!file_exists(paths.compress_exe)) {
        check.ok = false;
        check.error = "No existe el ejecutable compress_leaves: " + paths.compress_exe.string();
        return check;
    }

    return check;
}

std::uint32_t read_node_count(const fs::path& graph_path)
{
    std::ifstream input(graph_path, std::ios::binary);
    if (!input.is_open()) {
        return 0U;
    }

    std::uint32_t nodes = 0;
    input.read(reinterpret_cast<char*>(&nodes), sizeof(nodes));
    if (!input) {
        return 0U;
    }

    return nodes;
}

int max_real_level1_for(std::uint32_t nodes, int k1)
{
    if (nodes == 0 || k1 <= 1) {
        return 0;
    }

    return static_cast<int>(std::ceil(std::log(static_cast<double>(nodes)) /
                                      std::log(static_cast<double>(k1)))) - 1;
}

bool max_level1_is_valid(std::uint32_t nodes, int k1, int max_level1)
{
    return max_level1 <= max_real_level1_for(nodes, k1);
}
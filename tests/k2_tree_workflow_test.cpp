#include "tree_builder.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <set>

namespace {

void write_graph(const fs::path& path)
{
    fs::create_directories(path.parent_path());

    std::ofstream output(path, std::ios::out | std::ios::trunc);
    output << 5 << '\n';
    output << 5 << '\n';
    output << "0 1\n";
    output << "0 3\n";
    output << "1 2\n";
    output << "2 4\n";
    output << "3 4\n";
}

std::set<std::uint32_t> as_set(const std::vector<std::uint32_t>& values)
{
    return std::set<std::uint32_t>(values.begin(), values.end());
}

} // namespace

int main()
{
    const fs::path graph_path = fs::path("cds") / "k2_trees" / "tests" / "small_graph.pg";
    write_graph(graph_path);

    K2TreePreset preset = K2TreeBuilder::small();
    preset.max_level1 = 1;
    preset.s = 3;

    K2TreeBuilder builder(graph_path, preset, fs::path("cds") / "k2_trees" / "tests");

    if (!builder.build()) {
        std::cerr << "build failed\n";
        return 1;
    }

    const auto inv_before = builder.inverse(4);
    std::cerr << "inverse(4):";
    for (auto v : inv_before) {
        std::cerr << ' ' << v;
    }
    std::cerr << '\n';
    
    const auto adj_before = builder.adjacency(0);
    std::cerr << "adjacency(0):";
    for (auto v : adj_before) {
        std::cerr << ' ' << v;
    }
    std::cerr << '\n';

    std::cerr << "contains(2,4): " << builder.contains(2,4) << '\n';
    std::cerr << "contains(3,4): " << builder.contains(3,4) << '\n';

    if (!builder.saveCompressed()) {
        std::cerr << "saveCompressed failed\n";
        return 1;
    }

    const fs::path base = builder.artifact_base();
    assert(fs::exists(base.string() + ".tr"));
    assert(fs::exists(base.string() + ".lv"));
    assert(fs::exists(base.string() + ".il"));
    assert(fs::exists(base.string() + ".voc"));
    assert(fs::exists(base.string() + ".cil"));

    if (!builder.reloadCompressed()) {
        std::cerr << "reloadCompressed failed\n";
        return 1;
    }

    const auto inv_after = builder.inverse(4);
    std::cerr << "inverse(4):";
    for (auto v : inv_after) {
        std::cerr << ' ' << v;
    }
    std::cerr << '\n';

    const auto adj_after = builder.adjacency(0);
    std::cerr << "adjacency(0):";
    for (auto v : adj_after) {
        std::cerr << ' ' << v;
    }
    std::cerr << '\n';

    std::cerr << "contains(2,4): " << builder.contains(2,4) << '\n';
    std::cerr << "contains(3,4): " << builder.contains(3,4) << '\n';

    const auto adjacency_zero_debug = builder.adjacency(0);
    std::cerr << "adjacency(0):";
    for (auto value : adjacency_zero_debug) {
        std::cerr << ' ' << value;
    }
    std::cerr << '\n';

    const auto inverse_four_debug = builder.inverse(4);
    std::cerr << "inverse(4):";
    for (auto value : inverse_four_debug) {
        std::cerr << ' ' << value;
    }
    std::cerr << '\n';

    if (!builder.contains(0, 1) || !builder.contains(0, 3) || builder.contains(1, 0) ||
        !builder.contains(1, 2) || !builder.contains(2, 4) || !builder.contains(3, 4) ||
        builder.contains(4, 3)) {
        std::cerr << "contains check failed\n";
        return 1;
    }

    const auto adjacency_zero = as_set(builder.adjacency(0));
    if (!(adjacency_zero.count(1) == 1 && adjacency_zero.count(3) == 1 && adjacency_zero.size() == 2)) {
        std::cerr << "adjacency(0) check failed\n";
        return 1;
    }

    const auto inverse_four = as_set(builder.inverse(4));
    if (!(inverse_four.count(2) == 1 && inverse_four.count(3) == 1 && inverse_four.size() == 2)) {
        std::cerr << "inverse(4) check failed\n";
        return 1;
    }

    const auto adjacency_one = as_set(builder.adjacency(1));
    if (!(adjacency_one.count(2) == 1 && adjacency_one.size() == 1)) {
        std::cerr << "adjacency(1) check failed\n";
        return 1;
    }

    // degree()
    if (builder.degree(0) != 2 ||
        builder.degree(1) != 1 ||
        builder.degree(2) != 1 ||
        builder.degree(3) != 1 ||
        builder.degree(4) != 0) {
        std::cerr << "degree check failed\n";
        return 1;
    }

    // neighbors()
    if (!builder.neighbors(0, 1) ||
        !builder.neighbors(0, 3) ||
        !builder.neighbors(1, 2) ||
        !builder.neighbors(2, 4) ||
        !builder.neighbors(3, 4) ||
        builder.neighbors(1, 0) ||
        builder.neighbors(4, 2) ||
        builder.neighbors(4, 3)) {
        std::cerr << "neighbors check failed\n";
        return 1;
    }

    std::cout << "k2_tree workflow test passed\n";
    return 0;
}

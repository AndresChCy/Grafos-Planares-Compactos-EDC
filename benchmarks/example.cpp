#include <cassert>
#include <string>
#include <vector>
#include <random>

#include "bench-lib/benchmark.hpp"
#include "sdsl/pemb.hpp"
#include "complementary/Graph.hpp"
#include "complementary/utils.hpp"

#include "fib-lib/fib_tabulated.hpp"
#include "fib-lib/fib_memoized.hpp"
#include "fib-lib/fib_recursive.hpp" 

inline void test_degree_pemb(pemb<>& pe, std::vector<int>& vertices){
    for (int i = 0; i < vertices.size(); ++i) {
        pe.degree(vertices[i]);
    }
}

int main() {
  pemb<>* pe = nullptr;
  std::vector<std::string> archivos = {"benchmarks/inputs/planar_embedding5000000.pg"};
  std::random_device rd;
	std::mt19937 gen(rd());

  std::string csv_name = "example_res";
  for(size_t i = 0; i < archivos.size(); ++i) {
    {
      Graph g = read_graph_from_file(archivos[i].c_str());
      pe = new pemb<>(g);
    }
    std::uniform_int_distribution<> dis(0, pe->vertices() - 1); //Para generar vertices aleatorios
    std::vector<int> vertices;
    vertices.reserve(1000);
    for(int j = 0; j < 1000; ++j){
        vertices.push_back(dis(gen));
    }
    assert(vertices.size() == 1000);


    BenchLib::Benchmark bench;

    bench.add("degree_pemb", [&pe, &vertices]() {
      test_degree_pemb(*pe,vertices);
    }).set_label(archivos[i]);

    bench.run(32,16);

    if(i == 0) bench.write_csv(csv_name);
		else bench.append_csv(csv_name);

    delete pe;
    pe = nullptr;
  }

  return 0;
}
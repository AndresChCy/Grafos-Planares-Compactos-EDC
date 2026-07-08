#include <cassert>
#include <string>
#include <vector>
#include <random>
#include <

#include "bench-lib/benchmark.hpp"
#include "sdsl/pemb.hpp"
#include "complementary/Graph.hpp"
#include "complementary/utils.hpp"
#include "turan/neighbours.h"
#include "tree_builder.hpp"

const string PATH = "benchmarks/inputs/";

inline void test_degree_pemb(pemb<>& pe, std::vector<int>& vertices){
    for (int i = 0; i < vertices.size(); ++i) {
        pe.degree(vertices[i]);
    }
}

inline void test_neighbour_pemb(pemb<>& pe, std::vector<int>& vertices){
    int loop = vertices.size()/ 2;
    for (int i= 0; i < loop; i++){
      neighbours(pe, vertices[i], vertices[loop+i]);
    }
}


inline void test_degree_k2tree(K2TreeBuilder& graph, std::vector<int>& vertices){
    for (int i = 0; i < vertices.size(); ++i) {
        graph.degree(vertices[i]);
    }
}

inline void test_neighbour_k2tree(K2TreeBuilder& graph, std::vector<int>& vertices){
    int loop = vertices.size()/ 2;
    for (int i= 0; i < loop; i++){
      graph.neighbors(vertices[i], vertices[loop+i]);
    }
}

inline void test_neighbours_graph(Graph& g, std::vector<int>& vertices ){
  int loop = vertices.size()/ 2;
    for (int i= 0; i < loop; i++){
      g.neighbours(vertices[i],vertices[loop-i]);
    }
}

inline void test_degree_graph(Graph& g, std::vector<int>& vertices){
   for (int i= 0; i < vertices.size(); i++){
      g.degree(vertices[i]);
    }
}

int main(int argc, char* argv[]) {
  pemb<>* pe = nullptr;
  fs::path output_root = (argc > 2) ? fs::path(argv[2]) : fs::path("cds/k2_trees");
  //Los archivos deben ir en una carpeta dentro de benchmarks llamado inputs, es decir /benchmarks/inputs
  std::vector<std::string> archivos = {"planar_embedding5000000.pg", "PON OTRO ARCHIVO AQUI ", "Y OTRO MA"};
  std::random_device rd;
	std::mt19937 gen(rd());

  std::string csv_name = "degree_and_neighbours";
  const std::vector<K2TreePreset> presets = {
        K2TreeBuilder::small(),
        K2TreeBuilder::large(),
    };

  for(size_t i = 0; i < archivos.size(); ++i) {
    K2TreeBuilder k2_tree_small(PATH + archivos[i], presets[0], output_root);
    if (!k2_tree_small.build()) {
            std::cerr << "Fallo la construccion de " << presets[0].name << '\n';
            continue;
        }
    
    K2TreeBuilder k2_tree_large(PATH + archivos[i], presets[1], output_root);    
    if (!k2_tree_large.build()) {
            std::cerr << "Fallo la construccion de " << presets[1].name << '\n';
            continue;
        }
    { 
      //Por lo que vi al usar el grafo en el constructor su contenido se modifica por lo que 
      //este solo lo usamos para la destruccion
      Graph g_aux = read_graph_from_file( (PATH +archivos[i]).c_str());
      pe = new pemb<>(g_aux);
    }
    
    Graph g = read_graph_from_file((PATH +archivos[i]).c_str());
    
    
    std::uniform_int_distribution<> dis(0, pe->vertices() - 1); //Para generar vertices aleatorios
    std::vector<int> vertices;
    vertices.reserve(2000);
    for(int j = 0; j < 2000; ++j){
        vertices.push_back(dis(gen));
    }
    assert(vertices.size() == 2000);

    assert(pe->degree(0) == k2_tree_small.degree(0));
    assert(neighbours(*pe,0,1) == k2_tree_large.neighbors(0,1));
    assert(k2_tree_large.neighbors(0,1) == g.neighbours(0,1));
    
    BenchLib::Benchmark bench;

    bench.add("degree_pemb", [&pe, &vertices]() {
      test_degree_pemb(*pe,vertices);
    }).set_label(archivos[i]).set_size_in_megabytes(size_in_bytes(*pe)/(1024 * 1024));

    bench.add("neighbour_pemb", [&pe, &vertices](){
      test_neighbour_pemb(*pe,vertices);
    }).set_label(archivos[i]).set_size_in_megabytes(size_in_bytes(*pe)/(1024 * 1024));

    bench.add("degree_k2tree_small", [&k2_tree_small, &vertices](){
      test_degree_k2tree(k2_tree_small, vertices);
    }).set_label(archivos[i]).set_size_in_megabytes(k2_tree_small.compressionSizes().compressed_bytes 
  / (1024*1024));

    bench.add("degree_k2tree_large", [&k2_tree_large, &vertices](){
      test_neighbour_k2tree(k2_tree_large,vertices);
    }).set_label(archivos[i]).set_size_in_megabytes(k2_tree_large.compressionSizes().compressed_bytes
  / (1024*1024));

    bench.add("degree_ady_list", [&g,&vertices](){
      test_degree_graph(g,vertices);
    }).set_label(archivos[i]).set_size_in_megabytes((2*g.vertices()*sizeof(int) + g.edges()* (3*sizeof(int)
  + sizeof(bool)) + 2*sizeof(int))/(1024*1024) );

    bench.add("neighbours_ady_list", [&g,&vertices](){
      test_neighbours_graph(g,vertices);
    }).set_label(archivos[i]).set_size_in_megabytes((2*g.vertices()*sizeof(int) + g.edges()* (3*sizeof(int)
  + sizeof(bool)) + 2*sizeof(int))/(1024*1024) );
    
    bench.run(32,16);

    if(i == 0) bench.write_csv(csv_name);
		else bench.append_csv(csv_name);

    delete pe;
  
  }

  return 0;
}
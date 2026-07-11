#include <cassert>
#include <string>
#include <vector>
#include <random>

#include "bench-lib/benchmark.hpp"
#include "sdsl/pemb.hpp"
#include "complementary/Graph.hpp"
#include "complementary/utils.hpp"
#include "turan/neighbours.h"
//#include "tree_builder.hpp"
#include "k2_pg_io.h"
#include "k2_easy.h"
#include "k2_partition.h"

const string PATH = "benchmarks/inputs/";

inline void test_degree_pemb(pemb<>& pe, std::vector<int>& vertices){
    for (int i = 0; i < vertices.size(); ++i) {
        pe.degree(vertices[i]);
    }
}

inline void test_neighbour_pemb(pemb<>& pe, std::vector<int>& vertices){
    int loop = vertices.size()/ 2;
    for (int i= 0; i < loop; i++){
      neighbours(pe, vertices[i], 2);
    }
}

inline void test_neighbours_graph(Graph& g, std::vector<int>& vertices ){
  int loop = vertices.size()/ 2;
    for (int i= 0; i < loop; i++){
      g.neighbours(vertices[i],2);
    }
}

inline void test_degree_graph(Graph& g, std::vector<int>& vertices){
   for (int i= 0; i < vertices.size(); i++){
      g.degree(vertices[i]);
    }
}

inline void test_degree_k2tree(TREP* trep, std::vector<int>& vertices){
   for (int i= 0; i < vertices.size(); i++){
      degree(trep,i);
    }
}

inline void test_neighbours_k2tree(TREP * trep, std::vector<int>& vertices ){
  int loop = vertices.size()/ 2;
    for (int i= 0; i < loop; i++){
      neighbour(trep, vertices[i], 2);
    }
}

int main(int argc, char* argv[]) {
  pemb<>* pe = nullptr;
  //Los archivos deben ir en una carpeta dentro de benchmarks llamado inputs, es decir /benchmarks/inputs
  std::vector<std::string> archivos = { "planar_embedding1000000.pg","planar_embedding10000000.pg","planar_embedding5000000.pg"};
  std::random_device rd;
	std::mt19937 gen(rd());

  const char * tmpPrefix = "/tmp/k2bench_tmp"; //Archivos temporales de k2tree
  unsigned partitionS = 20;

  std::string csv_name = "degree_and_neighbours_nodo_fijo222222";
  for(size_t i = 0; i < archivos.size(); ++i) {
    TREP * trep;
    {
      PgGraph g;
      if (k2_read_pg_file((PATH+archivos[i]).c_str(), &g) != 0) {
          fprintf(stderr, "No se pudo leer %s\n", (PATH+archivos[i]).c_str());
          return 1;
      }
      if (k2_build_partitioned(g.numNodes, g.us, g.vs, g.numEdges, partitionS, tmpPrefix) != 0) {
            fprintf(stderr, "fallo el build particionado\n");
            k2_free_pg_graph(&g);
            return 1;
        }
        trep = k2_load((char *) tmpPrefix);
      
      k2_free_pg_graph(&g);
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
    vertices.reserve(5000);
    for(int j = 0; j < 5000; ++j){
        vertices.push_back(1);
    }
    assert(vertices.size() == 5000);
    assert(g.connected_graph());
    //assert(pe->degree(0) == k2_tree_small.degree(0));
    assert(g.neighbours(0,1) == neighbour(trep,0,1));
    assert(g.degree(1)== degree(trep,1));
    assert(neighbours(*pe,0,1) == neighbours(*pe,1,0));
    //assert(k2_tree_large.neighbors(0,2) == g.neighbours(0,2));
    
    BenchLib::Benchmark bench;

    bench.add("degree_pemb", [&pe, &vertices]() {
      test_degree_pemb(*pe,vertices);
    }).set_label(archivos[i]).set_size_in_megabytes(size_in_bytes(*pe)/(1024 * 1024));

    bench.add("neighbour_pemb", [&pe, &vertices](){
      test_neighbour_pemb(*pe,vertices);
    }).set_label(archivos[i]).set_size_in_megabytes(size_in_bytes(*pe)/(1024 * 1024));

    bench.add("degree_k2tree", [&trep, &vertices](){
      test_degree_k2tree(trep,vertices);
    }).set_label(archivos[i]).set_size_in_megabytes(k2_disk_size_mb(tmpPrefix));

    bench.add("neighbour_k2tree", [&trep, &vertices](){
      test_neighbours_k2tree(trep,vertices);
    }).set_label(archivos[i]).set_size_in_megabytes(k2_disk_size_mb(tmpPrefix));

  
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
    k2_destroy(trep);
  
  }

  return 0;
}
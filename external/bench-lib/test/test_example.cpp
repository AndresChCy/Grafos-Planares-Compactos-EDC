#include <cassert>
#include <random>
#include "bench-lib/benchmark.hpp"
#include "bench-lib/benchmark_manager.hpp"

int main() {
  std::random_device rd;
	std::mt19937 gen(rd());
  //Ejemplo de pruebas más clásicas, como saber cuál es el tiempo de una consulta X sobre una estructura de datos/algoritmo.
  //Por ejemplo, si queremos saber cuanto es el tiempo promedio para acceder a un elemento de un vector, no nos podemos quedar
  //únicamente con una ejecución (de 128 veces) de una tarea, si no que deberíamos repetirla sobre N índices diferentes escogidos
  //aleatoriamente. Aquí habrían dos posibles opciones de gráfico.
  
  //El primero sería tiempo versus índice del elemento a acceder, el cuál nos mostraría si varía el tiempo de acceso a un elemento
  //si vamos moviendo el índice a acceder, esto sería para un arreglo de tamaño fijo,
  {
    int n = 10'000'000;
    std::string csv_name = "random_access_"+std::to_string(n);
    std::vector<int> input(n);
    for(size_t i = 0; i < n; ++i)
      input[i] = i;
    std::uniform_int_distribution<> distrib(0, n - 1);
    for(int run = 0; run < 1000; ++run) {
      BenchLib::Benchmark bench;
      //esto podría no ser aleatorio si quieren, lo pueden dejar creciente para ver como se comporta, pero es un ejemplo
      int random_index = distrib(gen);
      bench.add("random access to position "+std::to_string(random_index), [&input](int _random_index) {
        return input[_random_index];
      }, random_index).set_input_size(random_index).set_label("rand_acc");
      //notar como deje el input_size como el índice a acceder, para poder usarlo como eje X en un gráfico.
      bench.run();

      int element = bench.get_result<int>(0);
      assert(element == random_index);
      if(run == 0) bench.write_csv(csv_name);
      else bench.append_csv(csv_name);
    }
  }

  //El segundo sería saber cual es el tiempo promedio para acceder a cualquier elemento de un arreglo, esta es más general y se pueden
  //probar diferentes tamaños de arreglo para ver si se mantiene constante este tiempo de acceso.
  //Otro ejemplo sería como sería el tiempo de consulta promedio de una búsqueda binaria a lo largo que se aumenta el tamaño del input
  //Otro ejemplo sería el tiempo promedio de responder una misma consulta, diferentes CDS en promedio y poder comparar el tiempo con el espacio
  {
    std::string csv_name = "search_experiment";
    std::vector<int> sizes = {100'000, 500'000, 1'000'000, 5'000'000, 10'000'000};
    for(size_t i = 0; i < sizes.size(); ++i) {
      //para este tipo de experimentos necesitamos al manager, que promediará la ejecución de los runs
      BenchLib::BenchmarkManager manager;
      int sz = sizes[i];
      std::uniform_int_distribution<> distrib(-10'000, sz+10'001);
      std::vector<int> input(sz);
      for(int x = 1; x <= sz; ++x)
        input[x-1] = x; 
      //input tendrá elementos [1,2,...,sz]'
      for(int run = 0; run < 1000; ++run) {
        BenchLib::Benchmark bench;
        int random_element = distrib(gen);
        bench.add("binary search", [&input](int _random_element) {
          //buscaremos el último índice talque input[mid] <= _random_element
          int L = -1, R = (int) input.size();
          while(R - L > 1) {
            int mid = L + (R-L)/2;
            if(input[mid] <= _random_element)
              L = mid;
            else
              R = mid;
          }
          return L;
        }, random_element).set_input_size(sz).set_label("bs");

        bench.add("sequential search", [&input](int _random_element) {
          //buscaremos el último índice talque input[L] <= _random_element
          int L = -1;
          for(size_t i = 0; i < input.size(); ++i) {
            if(input[i] <= _random_element)
              L = i;
            else
              break;
          }
          return L;
        }, random_element).set_input_size(sz).set_label("ls");
        bench.run();
        int res_bs = bench.get_result<int>(0);
        int res_ls = bench.get_result<int>(1);
        assert(res_bs == res_ls);
        manager.add_results(bench.get_tasks());
      }
      manager.average_by_task();
      if(i == 0) manager.write_csv(csv_name);
      else manager.append_csv(csv_name);
    }
  }
  return 0;
}
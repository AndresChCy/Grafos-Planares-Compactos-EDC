#include "bench-lib/benchmark_base.hpp"

namespace BenchLib {
  bool BenchmarkBase::append_csv(std::string csv_name) {
    std::ofstream file;
    file.open("results/"+csv_name+".csv", std::ios::app);
    if (!file.is_open()) {
      std::cerr << "Error: No se pudo abrir el archivo." << '\n';
      return false;
    }
    file.setf(std::ios_base::fixed);
    for(auto bc: tasks) {
      file << bc.explanation << ',' << bc.label << ',' << bc.input_size << ',' << bc.avg_time << ',' << bc.min_time << ',' << bc.max_time << ',' << bc.std_time << ',' << bc.size_in_megabytes << '\n';
    }
    file.close();
    return true;
  }
}
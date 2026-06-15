#include "bench-lib/benchmark_base.hpp"

namespace BenchLib {
  bool BenchmarkBase::write_csv(std::string csv_name) {
    std::filesystem::create_directories("results");
    std::ofstream file;
    file.open("results/"+csv_name+".csv");
    if (!file.is_open()) {
      std::cerr << "Error: No se pudo abrir el archivo." << '\n';
      return false;
    }
    file.setf(std::ios_base::fixed);
    file << "explanation,label,input_size,avg_t,min_t,max_t,std_t,size_in_megabytes\n";
    for(auto bc: tasks) {
      file << bc.explanation << ',' << bc.label << ',' << bc.input_size << ',' << bc.avg_time << ',' << bc.min_time << ',' << bc.max_time << ',' << bc.std_time << ',' << bc.size_in_megabytes << '\n';
    }
    file.close();
    return true;
  }
}
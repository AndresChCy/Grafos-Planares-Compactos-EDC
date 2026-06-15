#pragma once
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include "benchmark_case.hpp"

namespace BenchLib {
  class BenchmarkBase {
  protected:
    std::vector<BenchmarkCase> tasks;
  public:
    bool write_csv(std::string csv_name);
    bool append_csv(std::string csv_name);
  };
}
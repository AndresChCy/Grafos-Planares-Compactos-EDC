#pragma once

#include "benchmark_case.hpp"

namespace BenchLib {
  class BenchmarkBuilder {
  private:
    BenchmarkCase& c;
  public:
    BenchmarkBuilder(BenchmarkCase& c);
    BenchmarkBuilder& set_input_size(size_t _input_size);
    BenchmarkBuilder& set_label(std::string _label);
    BenchmarkBuilder& set_size_in_megabytes(double _size_in_megabytes);
  };
}
#include "bench-lib/benchmark_builder.hpp"

namespace BenchLib {
  BenchmarkBuilder::BenchmarkBuilder(BenchmarkCase& c) : c(c) {}

  BenchmarkBuilder& BenchmarkBuilder::set_input_size(size_t _input_size) {
      c.input_size = _input_size;
      return *this;
  }

  BenchmarkBuilder& BenchmarkBuilder::set_label(std::string _label) {
      c.label = _label;
      return *this;
  }

  BenchmarkBuilder& BenchmarkBuilder::set_size_in_megabytes(double _size_in_megabytes) {
      c.size_in_megabytes = _size_in_megabytes;
      return *this;
  }
}
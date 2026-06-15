#pragma once
#include "benchmark_base.hpp"
#include "benchmark_case.hpp"

namespace BenchLib {
  class BenchmarkManager: public BenchmarkBase {
  private:
    std::vector<std::vector<BenchmarkCase>> executed_tasks;
  public:
    void add_results(std::vector<BenchmarkCase> const &_executed_tasks);
    void average_by_task();
  };
}
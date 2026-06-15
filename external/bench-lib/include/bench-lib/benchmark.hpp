#pragma once

#include <vector>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "benchmark_builder.hpp"
#include "benchmark_base.hpp"

namespace BenchLib
{
  class Benchmark : public BenchmarkBase
  {
  private:
  public:
    template <typename F, typename... Args>
    BenchmarkBuilder add(const std::string &explanation, F &&f, Args &&...args) {
      auto args_tuple = std::make_tuple(std::forward<Args>(args)...);
      BenchmarkCase bc;
      bc.explanation = explanation;
      bc.func = [func = std::forward<F>(f), args_tuple]() mutable -> std::any {
        if constexpr (std::is_void_v<std::invoke_result_t<F, Args...>>) {
          std::apply(func, args_tuple);
          return std::any{};
        }
        else {
          return std::any(std::apply(func, args_tuple));
        }
      };
      tasks.push_back(std::move(bc));
      return BenchmarkBuilder(tasks.back());
    }

    template <typename T>
    T get_result(size_t index) const {
      return std::any_cast<T>(tasks[index].result);
    }

    std::vector<BenchmarkCase> get_tasks() {
      return tasks;
    }
    void run(int iterations = 128, int cache_warmup = 64);
  };
}
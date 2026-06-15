#pragma once

#include <functional>
#include <string>
#include <any>

namespace BenchLib {
  struct BenchmarkCase {
    std::string explanation;
    std::string label;
    std::function<std::any()> func;
    std::any result;
    size_t input_size = 0;
    double size_in_megabytes = 0;
    double avg_time = 0;
    double min_time = 0;
    double max_time = 0;
    double std_time = 0;
  };

}
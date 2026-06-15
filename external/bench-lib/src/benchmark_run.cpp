#include "bench-lib/benchmark.hpp"

namespace BenchLib {
  void Benchmark::run(int iterations, int cache_warmup) {
    using namespace std::chrono;
    for(auto &bc: tasks) {
      double min_t = 1'000'000'000'000'000'000, max_t = 0, total_t = 0, avg_t = 0, std_t = 0, micro_s = 0;
      std::any result;
      auto start = steady_clock::now();
      auto end = steady_clock::now();
      auto dur = end - start;
      std::chrono::duration<double, std::micro> micro_s_dur = dur;
      std::vector<double> internal_times(iterations);
      for(size_t cache = 0; cache < cache_warmup; ++cache) {
        bc.func();
      }
      for(size_t run = 0; run < iterations; ++run) {
        start = steady_clock::now();
        result = bc.func();
        end = steady_clock::now();
        dur = end - start;
        micro_s_dur = dur;
        micro_s = micro_s_dur.count();
        max_t = std::max(max_t, micro_s);
        min_t = std::min(min_t, micro_s);
        total_t += micro_s;
        internal_times[run] = micro_s; 
      }
      avg_t = total_t/iterations;
      bc.max_time = max_t;
      bc.min_time = min_t;
      bc.avg_time = avg_t;
      bc.result = result;
      for(size_t run = 0; run < iterations; ++run) {
        std_t += (internal_times[run] - avg_t) * (internal_times[run] - avg_t);
      }
      std_t /= iterations;
      std_t = std::sqrt(std_t);
      bc.std_time = std_t;
      std::cout << bc.explanation << " with input size " << bc.input_size << " took " << avg_t << " us\n";
    }
  }
}

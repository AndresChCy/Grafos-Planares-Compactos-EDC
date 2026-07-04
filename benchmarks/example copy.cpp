#include <cassert>
#include <string>

#include "bench-lib/benchmark.hpp"
#include "sdsl/pemb.hpp"
#include "fib-lib/fib_tabulated.hpp"
#include "fib-lib/fib_memoized.hpp"
#include "fib-lib/fib_recursive.hpp" 

int main() {
  std::vector<std::string> archivos = {10, 20, 30, 40};
  FibLib::FibRecursive fr;
  FibLib::FibMemoized fm;
  FibLib::FibTabulated ft;
  std::string csv_name = "example_res";
  for(size_t i = 0; i < sizes.size(); ++i) {
    int sz = sizes[i];
    BenchLib::Benchmark bench;
    bench.add("fib_rec", [&fr, sz]() {
      return fr.calc_fib(sz);
    }).set_input_size(sz).set_label("fr");

    bench.add("fib_mem", [&fm, sz]() {
      return fm.calc_fib(sz);
    }).set_input_size(sz).set_label("fm");

    bench.add("fib_tab", [&ft, sz]() {
      return ft.calc_fib(sz);
    }).set_input_size(sz).set_label("ft");

    bench.run();
    uint64_t r1 = bench.get_result<uint64_t>(0);
		uint64_t r2 = bench.get_result<uint64_t>(1);
		uint64_t r3 = bench.get_result<uint64_t>(2);
    assert(r1 == r2);
    assert(r2 == r3);
    if(i == 0) bench.write_csv(csv_name);
		else bench.append_csv(csv_name);
  }

  sizes = {50, 60, 70, 80, 85, 90, 91, 92, 93};
  for(auto sz: sizes) {

    BenchLib::Benchmark bench;
    bench.add("fib_mem", [&fm, sz]() {
      return fm.calc_fib(sz);
    }).set_input_size(sz).set_label("fm");

    bench.add("fib_tab", [&ft, sz]() {
      return ft.calc_fib(sz);
    }).set_input_size(sz).set_label("ft");

    bench.run();
    uint64_t r1 = bench.get_result<uint64_t>(0);
		uint64_t r2 = bench.get_result<uint64_t>(1);
    assert(r1 == r2);
    bench.append_csv(csv_name);
  }


  return 0;
}
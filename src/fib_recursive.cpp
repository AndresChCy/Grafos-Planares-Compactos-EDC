#include "fib-lib/fib_recursive.hpp" 

namespace FibLib {  

  uint64_t FibRecursive::calc_fib(uint8_t n) {
    if (n <= 1) return n;
    return calc_fib(n - 1) + calc_fib(n - 2);
  }

}
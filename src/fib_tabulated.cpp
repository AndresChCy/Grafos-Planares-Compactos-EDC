#include "fib-lib/fib_tabulated.hpp"

namespace FibLib {

  uint64_t FibTabulated::calc_fib(uint8_t n) {
    if (n <= 1) return n;
    uint64_t actual = 1, last = 0, aux;
    for (int i = 2; i <= n; ++i) {
      aux = actual;
      actual = actual + last;
      last = aux;
    }
    return actual;
  }

}
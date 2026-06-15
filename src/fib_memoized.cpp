#include "fib-lib/fib_memoized.hpp"

namespace FibLib {

  uint64_t FibMemoized::calc_fib(uint8_t n) {
    memo.assign(n + 1, -1);
    return fib(n);
  }

  uint64_t FibMemoized::fib(uint8_t n) {
    if (n <= 1) return n;
    if (memo[n] != -1)
        return memo[n];
    memo[n] = fib(n - 1) + fib(n - 2);
    return memo[n];
  }

}
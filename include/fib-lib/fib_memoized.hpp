#pragma once
#include "fib-lib/fib.hpp"

namespace FibLib {

  class FibMemoized : public Fib {
    private:
      std::vector<uint64_t> memo;
      uint64_t fib(uint8_t n);
    public:
      uint64_t calc_fib(uint8_t n) override;
  };

}
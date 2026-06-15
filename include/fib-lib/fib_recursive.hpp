#pragma once
#include "fib-lib/fib.hpp"

namespace FibLib {

  class FibRecursive : public Fib {
    public:
      uint64_t calc_fib(uint8_t n) override;
  };

}
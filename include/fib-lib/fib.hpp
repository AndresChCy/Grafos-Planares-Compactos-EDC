#pragma once

#include <vector>
#include <cstdint>

namespace FibLib {

  class Fib {
    public:
      virtual ~Fib() = default;

      virtual uint64_t calc_fib(uint8_t n) = 0;
  };

}
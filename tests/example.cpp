#include <cassert>
#include "fib-lib/fib_tabulated.hpp"
#include "fib-lib/fib_memoized.hpp"
#include "fib-lib/fib_recursive.hpp" 

int main() {
  FibLib::FibRecursive fr;
  FibLib::FibMemoized fm;
  FibLib::FibTabulated ft;

  assert(fr.calc_fib(35) == fm.calc_fib(35));
  assert(fm.calc_fib(35) == ft.calc_fib(35));

  assert(fm.calc_fib(80) == ft.calc_fib(80));

  assert(fr.calc_fib(0) == ft.calc_fib(0));
  assert(fm.calc_fib(0) == ft.calc_fib(0));
  assert(fr.calc_fib(0) == 0);

  assert(fr.calc_fib(1) == ft.calc_fib(1));
  assert(fm.calc_fib(1) == ft.calc_fib(1));
  assert(fr.calc_fib(1) == 1);

  assert(fr.calc_fib(2) == ft.calc_fib(2));
  assert(fm.calc_fib(2) == ft.calc_fib(2));
  assert(fr.calc_fib(2) == 1);

  assert(fr.calc_fib(8) == ft.calc_fib(8));
  assert(fm.calc_fib(8) == ft.calc_fib(8));
  assert(fr.calc_fib(8) == 21);
  return 0;
}
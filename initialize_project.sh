#!/bin/bash

git submodule add https://github.com/bletelier/bench-lib.git external/bench-lib
git submodule update --init --recursive

chmod +x compile.sh
chmod +x execute_benchmarks.sh
chmod +x execute_tests.sh
chmod +x plot.sh

cmake -S . -B build
./compile.sh
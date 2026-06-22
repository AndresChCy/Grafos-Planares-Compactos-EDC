cmake_minimum_required(VERSION 3.15)
project(my-project VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(my-project STATIC
  src/fib_recursive.cpp
  src/fib_memoized.cpp
  src/fib_tabulated.cpp
)

add_library(k2_tree
  src/k2_tree/basic.c
  src/k2_tree/bitrankw32int.c
  src/k2_tree/directcodes.c
  src/k2_tree/hash.c
  src/k2_tree/kTree.c
  src/k2_tree/MemoryManager.c
  src/k2_tree/grid_search.cpp
)


set(BENCHLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(external/bench-lib)

target_include_directories(my-project PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(k2_tree PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include/k2_tree
)

option(BUILD_TESTING "Build tests" ON)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()

option(BUILD_BENCHMARKS "Build benchmarks" ON)
if(BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()

target_compile_options(my-project PRIVATE -O3)
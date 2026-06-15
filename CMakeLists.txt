cmake_minimum_required(VERSION 3.15)
project(my-project VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(my-project STATIC
  src/fib_recursive.cpp
  src/fib_memoized.cpp
  src/fib_tabulated.cpp
)
set(BENCHLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(external/bench-lib)

target_include_directories(my-project PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
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
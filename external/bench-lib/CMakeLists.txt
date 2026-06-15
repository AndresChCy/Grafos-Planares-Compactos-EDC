cmake_minimum_required(VERSION 3.15)
project(bench-lib VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(bench-lib STATIC
    src/benchmark_run.cpp
    src/benchmark_base_write_csv.cpp
    src/benchmark_base_append_csv.cpp
    src/benchmark_builder.cpp
    src/benchmark_manager.cpp
)

target_include_directories(bench-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

option(BENCHLIB_BUILD_TESTING "Build BenchLib tests" ON)
if(BENCHLIB_BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

target_compile_options(bench-lib PRIVATE -O3)

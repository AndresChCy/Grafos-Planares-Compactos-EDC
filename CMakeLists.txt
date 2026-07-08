cmake_minimum_required(VERSION 3.15)
project(my-project VERSION 1.0 LANGUAGES C CXX)

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
  src/k2_tree/strcomp.c
  src/k2_tree/grid_search.cpp
  src/k2_tree/tree_builder.cpp
)

add_executable(build_tree
  src/k2_tree/build_tree_main.c
)

add_executable(compress_leaves
  src/k2_tree/compress_leaves_main.c
)


set(BENCHLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(external/bench-lib)
add_subdirectory(external/sdsl)

target_include_directories(my-project PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(k2_tree PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include/k2_tree
)

target_compile_options(k2_tree PRIVATE -fcommon)

target_link_libraries(build_tree PRIVATE k2_tree)
target_link_libraries(compress_leaves PRIVATE k2_tree)

set_target_properties(build_tree compress_leaves PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
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
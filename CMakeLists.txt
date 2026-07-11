cmake_minimum_required(VERSION 3.15)
project(my-project VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(my-project STATIC
  src/neighbours_turan.cpp
)

#add_compile_options(-fcommon -fgnu89-inline)

# --- Librería con el k2-tree legado + los helpers k2_easy ------------------
add_library(k2tree_legacy STATIC
    src/kTree.c
    src/hash.c
    src/MemoryManager.c
    src/bitrankw32int.c
    src/directcodes.c
    src/basic.c
    src/k2_easy.c
    src/k2_pg_io.c
    src/k2_partition.c
)

target_include_directories(k2tree_legacy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(k2tree_legacy PUBLIC m)


set(BENCHLIB_BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(external/bench-lib)
add_subdirectory(external/sdsl)

target_include_directories(my-project PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(k2tree_legacy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


#target_compile_options(k2_tree_legacy PRIVATE -fcommon -fgnu89-inline -O3)

#target_link_libraries(build_tree PRIVATE k2_tree)
#target_link_libraries(compress_leaves PRIVATE k2_tree)
target_link_libraries(my-project PRIVATE sdsl)
target_link_libraries(k2tree_legacy PUBLIC m)

#set_target_properties(build_tree compress_leaves PROPERTIES
#  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
#)

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

target_compile_options(k2tree_legacy PRIVATE -fcommon -fgnu89-inline -O3)
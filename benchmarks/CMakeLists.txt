add_executable(bench_example example.cpp)
target_link_libraries(bench_example PRIVATE my-project bench-lib sdsl k2tree_legacy)
target_compile_options(bench_example PRIVATE -O3)

add_executable(bench_example2 bench_nodo_fijo.cpp)
target_link_libraries(bench_example2 PRIVATE my-project bench-lib sdsl k2tree_legacy)
target_compile_options(bench_example2 PRIVATE -O3)


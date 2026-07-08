add_executable(bench_example example.cpp)
target_link_libraries(bench_example PRIVATE my-project bench-lib sdsl)
target_compile_options(bench_example PRIVATE -O3)


add_executable(grid k2_grid.cpp)
target_link_libraries(grid PRIVATE my-project bench-lib k2_tree)
target_compile_options(grid PRIVATE -O3)

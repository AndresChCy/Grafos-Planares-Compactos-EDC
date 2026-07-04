add_executable(bench_example example.cpp)
target_link_libraries(bench_example PRIVATE my-project bench-lib sdsl)
target_compile_options(bench_example PRIVATE -O3)


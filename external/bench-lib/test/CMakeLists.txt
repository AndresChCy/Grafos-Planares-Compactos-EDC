add_executable(test_example test_example.cpp)
target_link_libraries(test_example PRIVATE bench-lib)
add_test(NAME bench_basic_test2 COMMAND test_example)
target_compile_options(test_example PRIVATE -O3)

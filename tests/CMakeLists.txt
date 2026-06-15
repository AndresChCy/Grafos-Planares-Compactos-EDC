add_executable(test_example example.cpp)
target_link_libraries(test_example PRIVATE my-project)
add_test(NAME basic_test_example COMMAND test_example)
target_compile_options(test_example PRIVATE -O3)

add_executable(test_example example.cpp)
target_link_libraries(test_example PRIVATE my-project)
add_test(NAME basic_test_example COMMAND test_example)
target_compile_options(test_example PRIVATE -O3)

add_executable(test_k2_tree k2_tree_workflow_test.cpp)
target_link_libraries(test_k2_tree PRIVATE k2_tree)
add_test(NAME k2_tree_workflow_test COMMAND test_k2_tree)
target_compile_options(test_k2_tree PRIVATE -O3)

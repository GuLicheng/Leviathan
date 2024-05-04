enable_testing()

add_executable(static_vector_test ${CMAKE_SOURCE_DIR}/leviathan/collections/static_vector_test.cpp)
target_link_libraries(static_vector_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME static_vector_test COMMAND static_vector_test)

add_executable(ring_buffer_test ${CMAKE_SOURCE_DIR}/leviathan/collections/ring_buffer_test.cpp)
target_link_libraries(ring_buffer_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME ring_buffer_test COMMAND ring_buffer_test)

add_executable(buffer_test ${CMAKE_SOURCE_DIR}/leviathan/collections/buffer_test.cpp)
target_link_libraries(buffer_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME buffer_test COMMAND buffer_test)

add_executable(py_hash_test ${CMAKE_SOURCE_DIR}/leviathan/collections/py_hash_test.cpp)
target_link_libraries(py_hash_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME py_hash_test COMMAND py_hash_test)

add_executable(skip_list_test ${CMAKE_SOURCE_DIR}/leviathan/collections/skip_list_test.cpp)
target_link_libraries(skip_list_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME skip_list_test COMMAND skip_list_test)

add_executable(avl_tree_test ${CMAKE_SOURCE_DIR}/leviathan/collections/avl_tree_test.cpp)
target_link_libraries(avl_tree_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME avl_tree_test COMMAND avl_tree_test)














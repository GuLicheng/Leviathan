enable_testing()

set(COLLECTIONS_DIRECTORY collectionsV2)

add_executable(binary_search_tree_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/tree/binary_search_tree_test.cpp)
target_link_libraries(binary_search_tree_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME binary_search_tree_test COMMAND binary_search_tree_test)

add_executable(avl_tree_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/tree/avl_tree_test.cpp)
target_link_libraries(avl_tree_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME avl_tree_test COMMAND avl_tree_test)

add_executable(sequence_container_interface_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/sequence_container_interface_test.cpp)
target_link_libraries(sequence_container_interface_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME sequence_container_interface_test COMMAND sequence_container_interface_test)

add_executable(associative_container_interface_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/associative_container_interface_test.cpp)
target_link_libraries(associative_container_interface_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME associative_container_interface_test COMMAND associative_container_interface_test)





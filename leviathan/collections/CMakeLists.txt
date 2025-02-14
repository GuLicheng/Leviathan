enable_testing()

set(COLLECTIONS_DIRECTORY collections)

add_executable(pyhash_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/hashtable/pyhash_test.cpp)
target_link_libraries(pyhash_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME pyhash_test COMMAND pyhash_test)

add_executable(skiplist_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/list/skiplist_test.cpp)
target_link_libraries(skiplist_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME skiplist_test COMMAND skiplist_test)

# add_executable(binary_search_tree_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/tree/binary_search_tree_test.cpp)
# target_link_libraries(binary_search_tree_test PRIVATE Catch2::Catch2WithMain)
# add_test(NAME binary_search_tree_test COMMAND binary_search_tree_test)

add_executable(avl_tree_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/tree/avl_tree_test.cpp)
target_link_libraries(avl_tree_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME avl_tree_test COMMAND avl_tree_test)

add_executable(red_black_tree_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/tree/red_black_tree_test.cpp)
target_link_libraries(red_black_tree_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME red_black_tree_test COMMAND red_black_tree_test)

add_executable(treap_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/tree/treap_test.cpp)
target_link_libraries(treap_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME treap_test COMMAND treap_test)

add_executable(sequence_container_interface_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/sequence_container_interface_test.cpp)
target_link_libraries(sequence_container_interface_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME sequence_container_interface_test COMMAND sequence_container_interface_test)

add_executable(associative_container_interface_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/associative_container_interface_test.cpp)
target_link_libraries(associative_container_interface_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME associative_container_interface_test COMMAND associative_container_interface_test)

add_executable(buffer_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/buffer_test.cpp)
target_link_libraries(buffer_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME buffer_test COMMAND buffer_test)

add_executable(ring_buffer_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/ring_buffer_test.cpp)
target_link_libraries(ring_buffer_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME ring_buffer_test COMMAND ring_buffer_test)

add_executable(static_vector_test ${CMAKE_SOURCE_DIR}/leviathan/${COLLECTIONS_DIRECTORY}/static_vector_test.cpp)
target_link_libraries(static_vector_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME static_vector_test COMMAND static_vector_test)


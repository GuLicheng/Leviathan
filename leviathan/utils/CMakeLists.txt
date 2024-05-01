enable_testing()

add_executable(layout_test ${CMAKE_SOURCE_DIR}/leviathan/utils/layout_test.cpp)
target_link_libraries(layout_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME layout_test COMMAND layout_test)

add_executable(named_tuple_test ${CMAKE_SOURCE_DIR}/leviathan/utils/named_tuple_test.cpp)
target_link_libraries(named_tuple_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME named_tuple_test COMMAND named_tuple_test)
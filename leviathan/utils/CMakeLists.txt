enable_testing()

add_executable(layout_test ${CMAKE_SOURCE_DIR}/leviathan/utils/layout_test.cpp)
target_link_libraries(layout_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_layout_test COMMAND layout_test)
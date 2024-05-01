enable_testing()

add_executable(fixed_string_test ${CMAKE_SOURCE_DIR}/leviathan/string/fixed_string_test.cpp)
target_link_libraries(fixed_string_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_fixed_string_test COMMAND fixed_string_test)

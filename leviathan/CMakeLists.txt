enable_testing()

add_executable(value_test ${CMAKE_SOURCE_DIR}/leviathan/value_test.cpp)
target_link_libraries(value_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME value_test COMMAND value_test)


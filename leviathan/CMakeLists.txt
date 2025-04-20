enable_testing()

add_executable(callable_test ${CMAKE_SOURCE_DIR}/leviathan/callable_test.cpp)
target_link_libraries(callable_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME callable_test COMMAND callable_test)


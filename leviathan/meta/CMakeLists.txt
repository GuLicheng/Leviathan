enable_testing()

add_executable(function_traits_test ${CMAKE_SOURCE_DIR}/leviathan/meta/function_traits_test.cpp)
add_test(NAME function_traits_test COMMAND function_traits_test)













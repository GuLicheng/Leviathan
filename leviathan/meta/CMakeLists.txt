enable_testing()

add_executable(function_traits_test ${CMAKE_SOURCE_DIR}/leviathan/meta/function_traits_test.cpp)
add_test(NAME function_traits_test COMMAND function_traits_test)

add_executable(meta_test ${CMAKE_SOURCE_DIR}/leviathan/meta/meta_test.cpp)
add_test(NAME meta_test COMMAND meta_test)











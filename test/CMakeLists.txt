# macro(add_catch_test arg1 arg2)
#     add_executable(arg1 arg2)
#     target_link_libraries(arg1 PRIVATE Catch2::Catch2WithMain)
#     add_test(NAME arg1 COMMAND arg1)
# endmacro(add_catch_test)


enable_testing()

add_executable(test_buffer ${CMAKE_SOURCE_DIR}/test/test_buffer.cpp)
target_link_libraries(test_buffer PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_buffer COMMAND test_buffer)

add_executable(test_fixed_string ${CMAKE_SOURCE_DIR}/test/test_fixed_string.cpp)
target_link_libraries(test_fixed_string PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_fixed_string COMMAND test_fixed_string)

add_executable(test_skip_list ${CMAKE_SOURCE_DIR}/test/test_skip_list.cpp)
target_link_libraries(test_skip_list PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_skip_list COMMAND test_skip_list)

add_executable(test_py_hash ${CMAKE_SOURCE_DIR}/test/test_py_hash.cpp)
target_link_libraries(test_py_hash PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_py_hash COMMAND test_py_hash)

add_executable(test_combination ${CMAKE_SOURCE_DIR}/test/test_combination.cpp)
target_link_libraries(test_combination PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_combination COMMAND test_combination)
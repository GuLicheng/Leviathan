enable_testing()

add_executable(vector_test ${CMAKE_SOURCE_DIR}/leviathan/math/vector_test.cpp)
target_link_libraries(vector_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME vector_test COMMAND vector_test)

add_executable(int128_test ${CMAKE_SOURCE_DIR}/leviathan/math/int128_test.cpp)
target_link_libraries(int128_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME int128_test COMMAND int128_test)

# add_executable(test_big_integer ${CMAKE_SOURCE_DIR}/leviathan/math/test_bigint.cpp)
# target_link_libraries(test_big_integer PRIVATE Catch2::Catch2WithMain)
# add_test(NAME test_big_integer COMMAND test_big_integer)


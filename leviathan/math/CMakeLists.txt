enable_testing()

add_executable(test_vector ${CMAKE_SOURCE_DIR}/leviathan/math/vector.cpp)
target_link_libraries(test_vector PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_vector COMMAND test_vector)

add_executable(test_big_integer ${CMAKE_SOURCE_DIR}/leviathan/math/bigint.cpp)
target_link_libraries(test_big_integer PRIVATE Catch2::Catch2WithMain)
# add_test(NAME test_big_integer COMMAND test_big_integer)


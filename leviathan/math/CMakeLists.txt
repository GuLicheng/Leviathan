add_executable(test_vector ${CMAKE_SOURCE_DIR}/leviathan/math/vector.cpp)
target_link_libraries(test_vector PRIVATE Catch2::Catch2WithMain)


enable_testing()

add_executable(winnow_test ${CMAKE_SOURCE_DIR}/winnow/winnow_test.cpp)
target_link_libraries(winnow_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME winnow_test COMMAND winnow_test)


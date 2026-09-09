enable_testing()

add_executable(winnow_test ${CMAKE_SOURCE_DIR}/winnow/winnow_test.cpp)
target_link_libraries(winnow_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME winnow_test COMMAND winnow_test)

add_executable(winnow_ini ${CMAKE_SOURCE_DIR}/winnow/winnow_ini.cpp)
target_link_libraries(winnow_ini PRIVATE Catch2::Catch2WithMain)
add_test(NAME winnow_ini COMMAND winnow_ini)
enable_testing()

add_executable(annotation_test ${CMAKE_SOURCE_DIR}/leviathan/annotations/annotation_test.cpp)
target_link_libraries(annotation_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME annotation_test COMMAND annotation_test)

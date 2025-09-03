enable_testing()

add_executable(nom_test ${CMAKE_SOURCE_DIR}/leviathan/nom/nom_test.cpp)
target_link_libraries(nom_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME nom_test COMMAND nom_test)


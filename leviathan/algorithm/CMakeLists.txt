enable_testing()

add_executable(combination_test ${CMAKE_SOURCE_DIR}/leviathan/algorithm/combination_test.cpp)
target_link_libraries(combination_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME combination_test COMMAND combination_test)

add_executable(tim_sort_test ${CMAKE_SOURCE_DIR}/leviathan/algorithm/tim_sort_test.cpp)
target_link_libraries(tim_sort_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME tim_sort_test COMMAND tim_sort_test)

add_executable(linear_search_test ${CMAKE_SOURCE_DIR}/leviathan/algorithm/linear_search_test.cpp)
target_link_libraries(linear_search_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME linear_search_test COMMAND linear_search_test)

add_executable(heap_test ${CMAKE_SOURCE_DIR}/leviathan/algorithm/heap_test.cpp)
target_link_libraries(heap_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME heap_test COMMAND heap_test)

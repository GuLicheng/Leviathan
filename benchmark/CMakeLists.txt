add_executable(benchmark_buffer ${CMAKE_SOURCE_DIR}/benchmark/benchmark_buffer.cpp)
target_link_libraries(benchmark_buffer PRIVATE Catch2::Catch2WithMain)

add_executable(benchmark_collection ${CMAKE_SOURCE_DIR}/benchmark/benchmark_collection.cpp)
target_link_libraries(benchmark_collection PRIVATE Catch2::Catch2WithMain)

add_executable(random_range ${CMAKE_SOURCE_DIR}/benchmark/random_range.cpp)

add_executable(benchmark_range ${CMAKE_SOURCE_DIR}/benchmark/benchmark_range.cpp)
target_link_libraries(benchmark_range PRIVATE Catch2::Catch2WithMain)
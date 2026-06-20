# enable_testing()

# add_executable(vector_test ${CMAKE_SOURCE_DIR}/leviathan/math/vector_test.cpp)
# target_link_libraries(vector_test PRIVATE Catch2::Catch2WithMain)
# add_test(NAME vector_test COMMAND vector_test)


enable_testing()

file(GLOB_RECURSE ALG_HEADER ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

# 匹配所有 *_test.cpp
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_LIST_DIR}/*_test.cpp)

foreach(test_src IN LISTS TEST_SOURCES)
    get_filename_component(test_exe ${test_src} NAME_WE)
    add_executable(${test_exe} ${test_src})
    # 只链接自己的算法库，Catch2单头无需link
    target_link_libraries(${test_exe} PRIVATE Catch2::Catch2WithMain)
    add_test(NAME ${test_exe} COMMAND ${test_exe})
endforeach()

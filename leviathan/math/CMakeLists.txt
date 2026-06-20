
enable_testing()

file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_LIST_DIR}/*_test.cpp)

foreach(test_src IN LISTS TEST_SOURCES)
    get_filename_component(test_exe ${test_src} NAME_WE)
    add_executable(${test_exe} ${test_src})
    target_link_libraries(${test_exe} PRIVATE Catch2::Catch2WithMain)
    add_test(NAME ${test_exe} COMMAND ${test_exe})
endforeach()

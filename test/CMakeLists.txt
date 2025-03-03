# enable_testing()

# file(GLOB TEST_SOURCES ./*.cpp)

# foreach(test ${TEST_SOURCES})
#     get_filename_component(TName ${test} NAME_WE)
#     message(${TName})
#     # add_test(NAME ${TName} COMMAND ${TName})    
#     set(SRCNAME "${TName}.cpp")
#     add_executable(${TName} ${CMAKE_SOURCE_DIR}/test/${SRCNAME})
#     target_link_libraries(${TName} PRIVATE Catch2::Catch2WithMain)
#     add_test(NAME ${TName} COMMAND ${TName})
# endforeach()



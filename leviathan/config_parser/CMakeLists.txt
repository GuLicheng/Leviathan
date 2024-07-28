enable_testing()

add_executable(ini ${CMAKE_SOURCE_DIR}/leviathan/config_parser/ini/ini.cpp)
target_link_libraries(ini PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_ini COMMAND ini)

add_executable(json_test ${CMAKE_SOURCE_DIR}/leviathan/config_parser/json/json_test.cpp)
target_link_libraries(json_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME json_test COMMAND json_test)

add_executable(json ${CMAKE_SOURCE_DIR}/leviathan/config_parser/json/json.cpp)
target_link_libraries(json PRIVATE Catch2::Catch2WithMain)
add_test(NAME json COMMAND json)

add_executable(toml ${CMAKE_SOURCE_DIR}/leviathan/config_parser/toml/toml.cpp)
target_link_libraries(toml PRIVATE Catch2::Catch2WithMain)
add_test(NAME test_toml COMMAND toml)

enable_testing()

# add_executable(ini ${CMAKE_SOURCE_DIR}/leviathan/config_parser/ini/ini.cpp)
# target_link_libraries(ini PRIVATE Catch2::Catch2WithMain)
# add_test(NAME test_ini COMMAND ini)

add_executable(json_test ${CMAKE_SOURCE_DIR}/leviathan/config_parser/json/json_test.cpp)
target_link_libraries(json_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME json_test COMMAND json_test)

add_executable(toml_test ${CMAKE_SOURCE_DIR}/leviathan/config_parser/toml/toml_test.cpp)
target_link_libraries(toml_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME toml_test COMMAND toml_test)

add_executable(json ${CMAKE_SOURCE_DIR}/leviathan/config_parser/json/json.cpp)
target_link_libraries(json PRIVATE Catch2::Catch2WithMain)
add_test(NAME json COMMAND json)

add_executable(toml ${CMAKE_SOURCE_DIR}/leviathan/config_parser/toml/toml.cpp)
target_link_libraries(toml PRIVATE Catch2::Catch2WithMain)
add_test(NAME toml COMMAND toml)

add_executable(value_cast_test ${CMAKE_SOURCE_DIR}/leviathan/config_parser/value_cast_test.cpp)
target_link_libraries(value_cast_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME value_cast_test COMMAND value_cast_test)

add_executable(nom_test ${CMAKE_SOURCE_DIR}/experimental/nom/test.cpp)
target_link_libraries(nom_test PRIVATE Catch2::Catch2WithMain)
add_test(NAME nom_test COMMAND nom_test)

add_executable(ini ${CMAKE_SOURCE_DIR}/leviathan/config_parser/ini.cpp)
target_link_libraries(ini PRIVATE Catch2::Catch2WithMain)

add_executable(json ${CMAKE_SOURCE_DIR}/leviathan/config_parser/json.cpp)
target_link_libraries(json PRIVATE Catch2::Catch2WithMain)


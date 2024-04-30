cmake_minimum_required(VERSION 3.10)

project(leviathan)

set(PROJECT_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE  "-O3 ")

include_directories(leviathan)
include_directories(thirdpart/Catch2/src)

add_subdirectory(thirdpart/Catch2)

add_executable(main main.cpp)

set(LEVIATHAN_BUILD_BENCHMARK ON)
set(LEVIATHAN_BUILD_TEST ON)


if(LEVIATHAN_BUILD_BENCHMARK STREQUAL "ON")
    message(STATUS "Build benchmark")
    add_subdirectory(benchmark)
endif()
    

if(LEVIATHAN_BUILD_TEST STREQUAL "ON")
    message(STATUS "Build test")
    add_subdirectory(test)
    include(CTest)
endif()

set(LEVIATHAN_BUILD_LOGGING ON)
if(LEVIATHAN_BUILD_LOGGING STREQUAL "ON")
    message(STATUS "Build logging")
    add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/logging)
endif()

set(LEVIATHAN_BUILD_CONFIG_PARSER ON)
if (LEVIATHAN_BUILD_CONFIG_PARSER STREQUAL "ON")
    message(STATUS "Build config parsers")
    add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/config_parser)
endif()

set(LEVIATHAN_BUILD_MATH ON)
if (LEVIATHAN_BUILD_MATH STREQUAL "ON")
    message(STATUS "Build math")
    add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/math)
endif()

set(LEVIATHAN_BUILD_ALGORITHM ON)
if (LEVIATHAN_BUILD_ALGORITHM STREQUAL "ON")
    message(STATUS "Build math")
    add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/algorithm)
endif()
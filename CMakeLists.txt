cmake_minimum_required(VERSION 3.10)

project(leviathan)

set(PROJECT_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE  "-O3 ")

include_directories(include)
include_directories(thirdpart/Catch2/src)

add_subdirectory(thirdpart/Catch2)
add_subdirectory(test)

include(CTest)
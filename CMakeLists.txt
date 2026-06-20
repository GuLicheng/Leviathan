cmake_minimum_required(VERSION 3.10)

project(leviathan)

set(PROJECT_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 26)

set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_ROOT_PATH}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_ROOT_PATH}/bin)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -freflection -Wno-unused")
set(CMAKE_CXX_FLAGS_RELEASE " -O3 -freflection -Wno-unused")

include_directories(${PROJECT_ROOT_PATH})

# Some third party libraries which are head-only
include_directories(${PROJECT_ROOT_PATH}/third_part/Catch2)
add_subdirectory(third_part/Catch2)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE "-lstdc++exp")


# Root CMakeLists.txt over


# if(LEVIATHAN_BUILD_BENCHMARK STREQUAL "ON")
#     message(STATUS "Build benchmark")
#     add_subdirectory(benchmark)
# endif()

# set(LEVIATHAN_BUILD_NOM ON)
# if(LEVIATHAN_BUILD_NOM STREQUAL "ON")
#     message(STATUS "Build nom")
#     add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/nom)
# endif()

# set(LEVIATHAN_BUILD_LOGGING ON)
# if(LEVIATHAN_BUILD_LOGGING STREQUAL "ON")
#     message(STATUS "Build logging")
#     add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/logging)
# endif()

# set(LEVIATHAN_BUILD_CONFIG_PARSER ON)
# if (LEVIATHAN_BUILD_CONFIG_PARSER STREQUAL "ON")
#     message(STATUS "Build config parsers")
#     add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/config_parser)
# endif()

set(LEVIATHAN_BUILD_MATH ON)
if (LEVIATHAN_BUILD_MATH STREQUAL "ON")
    message(STATUS "Build math")
    add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/math)
endif()

# set(LEVIATHAN_BUILD_ALGORITHM ON)
# if (LEVIATHAN_BUILD_ALGORITHM STREQUAL "ON")
#     message(STATUS "Build algorithm")
#     add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/algorithm)
# endif()

# set(LEVIATHAN_BUILD_ALGORITHM ON)
# if (LEVIATHAN_BUILD_ALGORITHM STREQUAL "ON")
#     message(STATUS "Build string")
#     add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/string)
# endif()

# set(LEVIATHAN_BUILD_ALGORITHM ON)
# if (LEVIATHAN_BUILD_ALGORITHM STREQUAL "ON")
#     message(STATUS "Build utilis")
#     add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/utils)
# endif()

# set(LEVIATHAN_BUILD_META ON)
# if (LEVIATHAN_BUILD_META STREQUAL "ON")
#     message(STATUS "Build meta")
#     add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/meta)
# endif()

# set(LEVIATHAN_BUILD_COLLECTIONS ON)
# if (LEVIATHAN_BUILD_COLLECTIONS STREQUAL "ON")
#     message(STATUS "Build collections")
#     add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/collections)
# endif()

# set(LEVIATHAN_BUILD_ANNOTATIONS ON)
# if (LEVIATHAN_BUILD_ANNOTATIONS STREQUAL "ON")
#     message(STATUS "Build annotations")
#     add_subdirectory(${PROJECT_ROOT_PATH}/leviathan/annotations)
# endif()

# set(LEVIATHAN_BUILD_ROOT ON)
# if (LEVIATHAN_BUILD_ROOT STREQUAL "ON")
#     message(STATUS "Build root")
#     add_subdirectory(${PROJECT_ROOT_PATH}/leviathan)
# endif()


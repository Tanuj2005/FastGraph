cmake_minimum_required(VERSION 3.16)
project(fastgraph CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall")

# Collect all src/ cpp files automatically
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(fastgraph ${SOURCES})
target_include_directories(fastgraph PRIVATE src/)

# Tests
enable_testing()
file(GLOB TEST_SOURCES "tests/*.cpp")
foreach(TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_include_directories(${TEST_NAME} PRIVATE src/)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
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

# Protocol source files needed by test_resp
set(PROTOCOL_SOURCES
    src/protocol/resp.cpp
    src/protocol/encoder.cpp
)

# KV source files needed by test_hashmap (for later)
set(KV_SOURCES
    src/kv/hashmap.cpp
    src/kv/ttl.cpp
    src/kv/kv_engine.cpp
)

# Map each test to the sources it needs
set(test_resp_EXTRA       ${PROTOCOL_SOURCES})
set(test_hashmap_EXTRA    ${KV_SOURCES})
set(GRAPH_SOURCES
    src/graph/dynamic_graph.cpp
    src/graph/csr_graph.cpp
    src/graph/graph_engine.cpp
    src/graph/bfs.cpp
    src/graph/dfs.cpp
    src/graph/dijkstra.cpp
)

set(test_graph_EXTRA ${GRAPH_SOURCES})
set(test_skiplist_EXTRA   "")
set(test_ttl_EXTRA        ${KV_SOURCES})
set(test_integration_EXTRA ${PROTOCOL_SOURCES} ${KV_SOURCES})

file(GLOB TEST_SOURCES "tests/*.cpp")
foreach(TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE} ${${TEST_NAME}_EXTRA})
    target_include_directories(${TEST_NAME} PRIVATE src/)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
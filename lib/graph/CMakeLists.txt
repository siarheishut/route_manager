cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 17)
project(Graph)

add_library(graph INTERFACE)
target_sources(graph PUBLIC
        FILE_SET HEADERS
        BASE_DIRS src
        FILES src/graph.h src/router.h)

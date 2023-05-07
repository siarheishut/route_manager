cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 17)
project(RootManager)

include(FetchContent)
include(GoogleTest)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(lib/json)

# root_manager config start
add_executable(root_manager
        src/main.cpp
        src/bus_manager.cpp
        src/request_processor.cpp
        src/request_parser.cpp)

target_link_libraries(root_manager json)
target_include_directories(root_manager PUBLIC .)
# root_manager config end

# tests start
add_executable(route_manager_tests
        src/request_parser.cpp
        src/request_processor.cpp
        src/bus_manager.cpp
        tests/request_parser_test.cpp
        tests/bus_manager_test.cpp
        tests/test_utils.cpp)

target_link_libraries(route_manager_tests GTest::gtest_main json)
target_include_directories(route_manager_tests PUBLIC . tests)
gtest_discover_tests(route_manager_tests)
# tests end

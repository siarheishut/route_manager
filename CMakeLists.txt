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

# root_manager config start
add_executable(root_manager
        src/main.cpp
        src/bus_manager.cpp
        src/utils.cpp
        src/request_processor.cpp
        src/request_reader.cpp)

target_include_directories(root_manager PUBLIC .)
# root_manager config end

# tests start
add_executable(route_manager_tests
        tests/request_reader_test.cpp
        tests/json_test.cpp
        tests/bus_manager_test.cpp
        tests/test_utils.cpp
        src/json/json.cpp
        src/request_reader.cpp
        src/utils.cpp
        src/request_processor.cpp
        src/bus_manager.cpp)

target_link_libraries(route_manager_tests GTest::gtest_main)
target_include_directories(route_manager_tests PUBLIC . tests)
gtest_discover_tests(route_manager_tests)
# tests end

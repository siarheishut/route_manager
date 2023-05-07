cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 17)
project(Json)

# json config start
add_library(json src/json.cpp)

target_include_directories(json PUBLIC include)
# json config end

# tests start
include(FetchContent)
include(GoogleTest)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(googletest)

add_executable(json_tests
        src/json.cpp
        tests/tests.cpp)

target_link_libraries(json_tests GTest::gtest_main)
target_include_directories(json_tests PUBLIC . include)
gtest_discover_tests(json_tests)
# tests end

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

FetchContent_Declare(
        svg
        GIT_REPOSITORY https://github.com/siarheishut/svg.git
        GIT_TAG v2.0
)
FetchContent_MakeAvailable(svg)

set(protobuf_BUILD_TESTS OFF)
set(ABSL_PROPAGATE_CXX_STD ON)
FetchContent_Declare(
        Protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG v25.0
)
FetchContent_MakeAvailable(Protobuf)

FetchContent_GetProperties(Protobuf SOURCE_DIR Protobuf_SOURCE_DIR)
include(${Protobuf_SOURCE_DIR}/cmake/protobuf-generate.cmake)

include_directories(
        ${Protobuf_SOURCE_DIR}/src
        ${Protobuf_SOURCE_DIR}/third_party/abseil-cpp
)

add_library(my_proto
        OBJECT
        ${CMAKE_SOURCE_DIR}/proto/transport_catalog.proto
)
target_link_libraries(my_proto PRIVATE libprotobuf)

target_include_directories(
        my_proto PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/proto>
        $<INSTALL_INTERFACE:include>
        ${INCLUDE_DIRECTORIES}
)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/proto)
protobuf_generate(
        TARGET my_proto
        LANGUAGE cpp
        IMPORT_DIRS ${CMAKE_SOURCE_DIR}/proto
        PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto
)

add_subdirectory(lib/json)
add_subdirectory(lib/graph)

# root_manager config start
add_executable(root_manager
        src/main.cpp
        src/bus_manager.cpp
        src/request_processor.cpp
        src/request_parser.cpp
        src/distance_computer.cpp
        src/sphere.cpp
        src/route_manager.cpp
        src/color_parser.cpp
        src/coords_converter.cpp
        src/map_renderer.cpp
        src/transport_catalog.cpp
)

target_link_libraries(root_manager PUBLIC json graph svg PRIVATE my_proto)
target_include_directories(root_manager PUBLIC .)
# root_manager config end

# tests start
add_executable(route_manager_tests
        src/request_parser.cpp
        src/request_processor.cpp
        src/bus_manager.cpp
        src/distance_computer.cpp
        src/sphere.cpp
        src/route_manager.cpp
        src/color_parser.cpp
        src/coords_converter.cpp
        src/map_renderer.cpp
        src/transport_catalog.cpp
        tests/request_parser_test.cpp
        tests/bus_manager_test.cpp
        tests/test_utils.cpp
        tests/request_processor_test.cpp
        tests/settings_parser_test.cpp
        tests/color_parser_test.cpp
        tests/map_renderer_test.cpp
        tests/coords_converter_test.cpp
        tests/transport_catalog_test.cpp
)

target_link_libraries(route_manager_tests GTest::gtest_main GTest::gmock_main json graph svg)
target_include_directories(route_manager_tests PUBLIC . tests)
gtest_discover_tests(route_manager_tests)
# tests end

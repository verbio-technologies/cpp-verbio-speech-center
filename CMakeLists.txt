cmake_minimum_required(VERSION 3.14)
project(cli_client VERSION 0.0.1)
include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")

find_package(cxxopts REQUIRED)
find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)
find_package(SndFile REQUIRED)
find_package(cpr REQUIRED)

add_subdirectory(proto)
add_subdirectory(src)

enable_testing()
include(CTest)
include(GoogleTest)
add_subdirectory(test)

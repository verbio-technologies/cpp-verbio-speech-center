cmake_minimum_required(VERSION 3.14)
project(cli_client VERSION 0.0.1)
include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

set(CMAKE_CXX_STANDARD 20)

option(USE_CXX11_ABI_0 "" ON)
if (USE_CXX11_ABI_0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
endif ()

find_package(cxxopts REQUIRED)
find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)
find_package(SndFile REQUIRED)
find_package(cpr REQUIRED)
find_package(jwt-cpp REQUIRED)
find_package(nlohmann_json REQUIRED)

add_subdirectory(proto)
add_subdirectory(src)

enable_testing()
include(CTest)
include(GoogleTest)
add_subdirectory(test)

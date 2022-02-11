cmake_minimum_required(VERSION 3.14)
project(cli_client VERSION 0.0.1)

string(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")


find_package(cxxopts REQUIRED)
find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)
find_package(SndFile REQUIRED)

include(cmake/ProtoTools.cmake)

protobuf_generate_grpc_cpp(SPEECH_CENTER_SOURCE SPEECH_CENTER_HEADER verbio-speech-center.proto)
message(STATUS "sources: ${SPEECH_CENTER_SOURCE}")
message(STATUS "headers: ${SPEECH_CENTER_HEADER}")

add_library(speech-center-grpc OBJECT
        ${SPEECH_CENTER_SOURCE}
        ${SPEECH_CENTER_HEADER}
        )
target_link_libraries(speech-center-grpc PUBLIC protobuf::protobuf gRPC::grpc++ gRPC::grpc++_reflection gRPC::gpr gRPC::grpc)
target_include_directories(speech-center-grpc PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_library(speech-center-client STATIC
        src/gRpcExceptions.cpp
        src/SpeechCenterClient.cpp
        src/Configuration.cpp
        )
target_link_libraries(speech-center-client PUBLIC  speech-center-grpc cxxopts::cxxopts spdlog::spdlog SndFile::sndfile)
target_include_directories(speech-center-client PUBLIC src)

add_executable(cli_client
        src/main.cpp)
target_link_libraries(cli_client PRIVATE speech-center-client)


enable_testing()
include(CTest)
include(GoogleTest)
add_subdirectory(test)

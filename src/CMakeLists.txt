
include_directories(${PROJECT_SOURCE_DIR}/headers)

add_library(speech-center-client STATIC
        gRpcExceptions.cpp
        RecognitionClient.cpp
        Configuration.cpp
        Audio.cpp
        SpeechCenterCredentials.cpp)

target_link_libraries(speech-center-client PUBLIC speech-center-grpc cxxopts::cxxopts spdlog::spdlog SndFile::sndfile cpr::cpr)

add_executable(cli_client
        main.cpp)
target_link_libraries(cli_client PRIVATE speech-center-client)
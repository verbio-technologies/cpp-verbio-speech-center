
find_package(Catch2 REQUIRED)
add_executable(test_commandLine
        test_commandLine.cpp)
target_link_libraries(test_commandLine PRIVATE speech-center-client Catch2::Catch2)
add_test(test_commandLine test_commandLine)

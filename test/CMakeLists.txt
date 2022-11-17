
include_directories(${PROJECT_SOURCE_DIR}/headers)

find_package(GTest REQUIRED)
add_executable(test_commandLine
        test_commandLine.cpp)
target_link_libraries(test_commandLine PRIVATE speech-center-client GTest::gtest_main GTest::gtest)
add_test(test_commandLine test_commandLine)

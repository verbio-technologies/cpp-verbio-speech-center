
include_directories(${PROJECT_SOURCE_DIR}/headers)

find_package(GTest REQUIRED)

function(add_unittest TARGET SOURCE)
    add_executable(${TARGET} ${SOURCE})
    target_link_libraries(${TARGET} PRIVATE speech-center-client GTest::gtest_main GTest::gtest)
    add_test(${TARGET} ${TARGET})
endfunction()

add_unittest(test_commandLine test_commandLine.cpp)
add_unittest(test_audio test_audio.cpp)
add_unittest(test_latencyStats test_latencyStats.cpp)

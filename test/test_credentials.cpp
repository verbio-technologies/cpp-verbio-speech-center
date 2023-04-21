#include <gtest/gtest.h>

#include "SpeechCenterCredentials.h"


TEST(credentials, first) {
    SpeechCenterCredentials credentials{""};

    auto expiration_time = credentials.decodeExpirationTime();

    ASSERT_EQ(expiration_time, 1681376822);
}
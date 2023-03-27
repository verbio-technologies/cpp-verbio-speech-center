

#include <gtest/gtest.h>

#include "Audio.h"

TEST(Audio, initialisationIsAllocation) {
    constexpr int numberOfSamples = 60000;
    std::array<int16_t, numberOfSamples> rawAudio;
    for (int i = 0; i < numberOfSamples; ++i) rawAudio[i] = i;
    Audio audio(rawAudio.data(), 8000, numberOfSamples);
    EXPECT_EQ(audio.getSamplingRate(), 8000);
    EXPECT_EQ(audio.getBytesPerSamples(), 2);
    EXPECT_EQ(audio.getLengthInBytes(), numberOfSamples * 2);
    EXPECT_EQ(audio.getLengthInFrames(), numberOfSamples);
    for (int i = 0; i < numberOfSamples; ++i)
        ASSERT_EQ(audio.getData()[i], rawAudio[i]);
}

TEST(Audio, chunks) {
    constexpr int numberOfSamples = 60000;
    constexpr int chunkLengthInSamples = 2203;
    std::array<int16_t, numberOfSamples> rawAudio;
    for (int i = 0; i < numberOfSamples; ++i) rawAudio[i] = i;
    auto chunks = Audio(rawAudio.data(), 8000, numberOfSamples).getAudioChunks(2203);
    EXPECT_EQ(chunks.size(), std::ceil(static_cast<float>(numberOfSamples) / chunkLengthInSamples));
    for (int chunk = 0; chunk < chunks.size(); ++chunk)
        for (int i = 0; i < chunkLengthInSamples; ++i)
                ASSERT_EQ(chunks[chunk].data()[i],
                          (chunkLengthInSamples*chunk + i < numberOfSamples) ? rawAudio[chunk*chunkLengthInSamples + i] : 0
                          ) << "i: " << i << ", chunk: " << chunk;
}

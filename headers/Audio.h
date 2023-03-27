#ifndef CLI_CLIENT_AUDIO_H
#define CLI_CLIENT_AUDIO_H

#include <fstream>
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>
#include <memory>


class Audio {
public:
    Audio(const int16_t *data, int samplingRate, int lengthInFrames);

    Audio(const std::string &audioPath);

    ~Audio();

    const int16_t *const getData() const { return data.get(); }

    int64_t getSamplingRate() const { return samplingRate; }

    int64_t getLengthInFrames() const { return length; }

    int64_t getLengthInBytes() const { return length * getBytesPerSamples(); }

    int64_t getBytesPerSamples() const { return sizeof(data[0]); }

    std::vector<std::vector<int16_t> > getAudioChunks(size_t chunkLength) const;

    int64_t getChannels() const { return channels; }

private:
    std::unique_ptr<int16_t[]> data;
    int64_t length{0};
    int64_t samplingRate{0};
    int64_t channels{1};
};


#endif //CLI_CLIENT_AUDIO_H


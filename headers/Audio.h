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
    Audio(const int16_t* data, int samplingRate, int lengthInFrames);
    ~Audio();
    const int16_t* const getData() const {return data.get();}
    int64_t getSamplingRate() const {return samplingRate;}
    int64_t getLengthInFrames() const {return length;}
    int64_t getLengthInBytes() const {return length*getBytesPerSamples();}
    int64_t getBytesPerSamples() const {return sizeof(data[0]);}

    template <std::size_t chunkLength>
    std::vector<std::array<int16_t, chunkLength> > getAudioChunks() const;
private:
    std::unique_ptr<int16_t[]> data;
    int64_t length{0};
    int64_t samplingRate{0};
};


class AudioFile {
public:
    explicit AudioFile(const std::string &audioPath);
    ~AudioFile();
    [[nodiscard]] const Audio& getAudio() const {return audio;}

private:
    Audio audio;
    static Audio readAudioFromFile(const std::string& audioPath);
};


template<std::size_t chunkLength>
std::vector<std::array<int16_t, chunkLength> > Audio::getAudioChunks() const {
    const auto numberOfChunks = static_cast<std::size_t >(std::ceil(static_cast<double>(length)  / chunkLength));
    std::vector<std::array<int16_t, chunkLength> > chunks(numberOfChunks, std::array<int16_t, chunkLength>());
    for (int i = 0; i < numberOfChunks - 1; ++i)
        std::copy_n(&data[i*chunkLength], chunkLength, chunks[i].begin());
    chunks[numberOfChunks - 1].fill(0);
    std::copy_n(&data[(numberOfChunks-1) * chunkLength], chunkLength - (numberOfChunks*chunkLength - length), chunks[numberOfChunks - 1].begin());
    return chunks;
}

#endif //CLI_CLIENT_AUDIO_H


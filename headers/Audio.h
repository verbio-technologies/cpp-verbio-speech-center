#ifndef CLI_CLIENT_AUDIO_H
#define CLI_CLIENT_AUDIO_H

#include <fstream>

class Audio {
public:
    explicit Audio(const std::string &audioPath);
    ~Audio();
    const int16_t* getData() const {return data;}
    int16_t* getData() {return data;}
    int64_t getLengthInFrames() const {return length;}
    int64_t getLengthInBytes() const {return length*getBytesPerSamples();}

private:
    int16_t *data{nullptr};
    int64_t length{0};

    int64_t getBytesPerSamples() const {return sizeof(*data);}
};


#endif //CLI_CLIENT_AUDIO_H

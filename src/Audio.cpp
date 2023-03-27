//
// Created by gserrahima on 28/02/23.
//

#include "Audio.h"

#include "gRpcExceptions.h"

#include "logger.h"
#include "sndfile.hh"

Audio::Audio(const int16_t *const _data, int _samplingRate, int lengthInFrames) : length(lengthInFrames),
                                                                                  samplingRate(_samplingRate) {
    data = std::make_unique<int16_t[]>(length);
    std::copy_n(_data, lengthInFrames, data.get());
}

Audio::~Audio() = default;

#include <iostream>

Audio::Audio(const std::string &audioPath) {
    SndfileHandle sndfileHandle(audioPath);
    auto length = sndfileHandle.frames();
    auto data = new int16_t[length];
    length = sndfileHandle.read(data, length);
    auto samplingRate = sndfileHandle.samplerate();
    if (!sndfileHandle.formatCheck(SF_FORMAT_PCM_16 | SF_FORMAT_WAV, sndfileHandle.channels(), samplingRate))
        throw GrpcException("Unsupported file audio format");
    INFO("Read {} samples with {} bytes per sample", length, sizeof(int16_t));

    this->length = length;
    this->samplingRate = samplingRate;
    this->channels = sndfileHandle.channels();
    this->data = std::make_unique<int16_t[]>(length);
    std::copy_n(data, length, this->data.get());
}

std::vector<std::vector<int16_t> > Audio::getAudioChunks(size_t chunkLength) const {

    const auto numberOfChunks = static_cast<std::size_t >(std::ceil(static_cast<double>(length) / chunkLength));
    if (numberOfChunks <= 0)
        return {};
    std::vector<std::vector<int16_t> > chunks(numberOfChunks, std::vector<int16_t>(chunkLength, 0));
    for (int i = 0; i < numberOfChunks - 1; ++i)
        std::copy_n(&data[i * chunkLength], chunkLength, chunks[i].begin());
    std::copy_n(&data[(numberOfChunks - 1) * chunkLength], chunkLength - (numberOfChunks * chunkLength - length),
                chunks[numberOfChunks - 1].begin());

    return chunks;
}

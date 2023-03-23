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
    this->length = sndfileHandle.frames();
    auto data = new int16_t[length];
    length = sndfileHandle.read(data, length);
    auto samplingRate = sndfileHandle.samplerate();
    if (!sndfileHandle.formatCheck(SF_FORMAT_PCM_16 | SF_FORMAT_WAV, sndfileHandle.channels(), samplingRate))
        throw GrpcException("Unsupported file audio format");
    INFO("Read {} samples with {} bytes per sample", length, sizeof(data));

    this->length = length;
    this->samplingRate = samplingRate;
    this->data = std::make_unique<int16_t[]>(length);
    std::copy_n(data, length, this->data.get());
}

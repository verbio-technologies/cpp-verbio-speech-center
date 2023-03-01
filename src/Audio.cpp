//
// Created by gserrahima on 28/02/23.
//

#include "Audio.h"

#include "logger.h"
#include "sndfile.hh"

Audio::Audio(const std::string &audioPath) {
    SndfileHandle sndfileHandle(audioPath);
    length = sndfileHandle.frames();
    data = new int16_t[length];
    length = sndfileHandle.read(data, length);
    INFO("Read {} samples with {} bytes per sample", sndfileHandle.frames(), getBytesPerSamples());
}

Audio::~Audio() {
    delete data;
}

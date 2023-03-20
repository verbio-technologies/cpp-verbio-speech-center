#ifndef ASRTEST_GRPCEXCEPTIONS_H
#define ASRTEST_GRPCEXCEPTIONS_H

#include <stdexcept>
#include <string>


class GrpcException : public std::runtime_error {
public:
    explicit GrpcException(const std::string &message);
};

class CallbackNotImplemented : public GrpcException {
public:
    CallbackNotImplemented();
};

class LanguageModelNotImplemented : public GrpcException {
public:
    explicit LanguageModelNotImplemented(const std::string &mode);
};

class MultipleLanguageModelsNotImplemented : public GrpcException {
public:
    MultipleLanguageModelsNotImplemented();
};

class UnknownTopicModel : public GrpcException {
public:
    explicit UnknownTopicModel(const std::string &topic);
};

class UnsupportedSampleRate : public GrpcException {
public:
    explicit UnsupportedSampleRate(const std::string &sampleRate);
};

class UnknownAsrVersion : public GrpcException {
public:
    explicit UnknownAsrVersion(const std::string &asrVersion);
};

class IOError : public GrpcException {
public:
    explicit IOError(const std::string &message);
};

class StreamException : public GrpcException {
public:
    explicit StreamException(const std::string message);
};

#endif

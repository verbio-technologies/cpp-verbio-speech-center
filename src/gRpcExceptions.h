#ifndef ASRTEST_GRPCEXCEPTIONS_H
#define ASRTEST_GRPCEXCEPTIONS_H

#include <string>
#include <stdexcept>


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

class IOError : public GrpcException {
public:
    explicit IOError(const std::string &message);
};

#endif

#include "gRpcExceptions.h"

GrpcException::GrpcException(const std::string &message) : std::runtime_error("GRPC gateway exception: " + message) {}

CallbackNotImplemented::CallbackNotImplemented() : GrpcException("Callback is not implemented for gRPC server") {}

UnknownTopicModel::UnknownTopicModel(const std::string &topic) : GrpcException("Unknown topic: " + topic) {}

UnknownGrammarModel::UnknownGrammarModel(const std::string &grammar) : GrpcException("Unknown grammar: " + grammar) {}

UnsupportedSampleRate::UnsupportedSampleRate(const std::string &sampleRate) : GrpcException(
        "Unsupported sample rate: " + sampleRate) {}

UnknownAsrVersion::UnknownAsrVersion(const std::string &asrVersion) : GrpcException("Invalid version: " + asrVersion) {}

LanguageModelNotImplemented::LanguageModelNotImplemented(const std::string &mode) : GrpcException(
        "Recognition of " + mode + " models is not implemented for gRPC backend.") {}

MultipleLanguageModelsNotImplemented::MultipleLanguageModelsNotImplemented() : GrpcException(
        "Multiple language models in parallel not implemented.") {}

StreamException::StreamException(const std::string message) : GrpcException("Stream error. Cannot write to stream. (" + message + ")") {}

IOError::IOError(const std::string &message) : GrpcException(message) {}

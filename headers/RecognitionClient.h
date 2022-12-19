#ifndef ASRTEST_RECOGNITIONCLIENT_H
#define ASRTEST_RECOGNITIONCLIENT_H

#include "Configuration.h"

#include <grpcpp/security/credentials.h>
#include <grpcpp/impl/codegen/client_context.h>
#include "recognition.grpc.pb.h"
#include "recognition.pb.h"

class Audio;

class RecognitionClient {
public:
    RecognitionClient(const Configuration &configuration);

    ~RecognitionClient();

    void connect(const Configuration& configuration);

private:
    std::unique_ptr<speechcenter::recognizer::v1::Recognizer::Stub> stub_;
    std::shared_ptr<grpc::Channel> channel;
    grpc::ClientContext context;

    static speechcenter::recognizer::v1::RecognitionResource_Model convertTopicModel(const std::string &modelName);

    static speechcenter::recognizer::v1::RecognitionStreamingRequest
        buildRecognitionConfig(const Configuration& configuration);

    static speechcenter::recognizer::v1::RecognitionStreamingRequest
        buildAudioRequest(const Configuration& configuration);

    static std::unique_ptr<speechcenter::recognizer::v1::RecognitionResource>
        buildRecognitionResource(const Configuration &configuration);

    static std::unique_ptr<speechcenter::recognizer::v1::RecognitionParameters>
        buildRecognitionParameters(const Configuration &configuration);

    static std::unique_ptr<speechcenter::recognizer::v1::PCM>
        buildPCM(const uint32_t &sampleRate);

    static std::string readFileContent(const std::string &path);
    static std::string sanitize(std::string str);

    std::shared_ptr<grpc::Channel>
    createChannel(const Configuration &configuration);
};

#endif
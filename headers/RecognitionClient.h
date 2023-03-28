#ifndef ASRTEST_RECOGNITIONCLIENT_H
#define ASRTEST_RECOGNITIONCLIENT_H

#include "Configuration.h"


#include "latencyStatistics.h"

#include "recognition.grpc.pb.h"
#include "recognition.pb.h"
#include <grpcpp/impl/codegen/client_context.h>
#include <grpcpp/security/credentials.h>


using namespace speechcenter::recognizer::v1;

class Audio;


class RecognitionClient {
public:
    explicit RecognitionClient(const Configuration &configuration);

    ~RecognitionClient();

    void connect(const Configuration &configuration);

private:
    std::unique_ptr<Recognizer::Stub> stub_;
    std::shared_ptr<grpc::Channel> channel;
    grpc::ClientContext context;
    Configuration configuration;
    LatencyLog latencyLog;

    static RecognitionResource_Topic convertTopic(const std::string &topicName);

    RecognitionStreamingRequest buildRecognitionConfig();

    std::vector<RecognitionStreamingRequest> buildAudioRequests();

    std::unique_ptr<RecognitionResource> buildRecognitionResource();

    std::unique_ptr<RecognitionParameters> buildRecognitionParameters();

    static std::unique_ptr<PCM> buildPCM(const uint32_t &sampleRate);

    RecognitionConfig_AsrVersion buildAsrVersion();

    static std::string readFileContent(const std::string &path);

    static std::string sanitize(std::string str);

    std::shared_ptr<grpc::Channel> createChannel();

    void
    write(std::shared_ptr<grpc::ClientReaderWriter<RecognitionStreamingRequest, RecognitionStreamingResponse>> stream);
};

#endif
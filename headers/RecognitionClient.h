#ifndef ASRTEST_RECOGNITIONCLIENT_H
#define ASRTEST_RECOGNITIONCLIENT_H

#include "Configuration.h"

#include "recognition.grpc.pb.h"
#include "recognition.pb.h"
#include <grpcpp/impl/codegen/client_context.h>
#include <grpcpp/security/credentials.h>

using namespace speechcenter::recognizer::v1;

typedef RecognitionStreamingRequest Request;
typedef RecognitionStreamingResponse Response;

class Audio;

class RecognitionClient {
public:
    RecognitionClient(const Configuration &configuration);

    ~RecognitionClient();

    void performStreamingRecognition();

private:
    std::unique_ptr<Recognizer::Stub> stub_;
    std::shared_ptr<grpc::Channel> channel;
    grpc::ClientContext context;
    Configuration configuration;

    static RecognitionResource_Topic convertTopic(const std::string &topicName);

    RecognitionStreamingRequest buildRecognitionConfig();

    std::vector<RecognitionStreamingRequest> buildAudioRequests();

    std::unique_ptr<RecognitionResource> buildRecognitionResource();

    std::unique_ptr<RecognitionParameters> buildRecognitionParameters();

    static std::unique_ptr<PCM> buildPCM(const uint32_t &sampleRate);

    RecognitionConfig_AsrVersion buildAsrVersion();

    std::shared_ptr<grpc::Channel> createChannel();

    void
    write(std::shared_ptr<grpc::ClientReaderWriter<RecognitionStreamingRequest, RecognitionStreamingResponse>> stream);

    std::string getJwtToken() const;

    void establishConnection(const std::string &jwt);

    std::shared_ptr<grpc::Channel> getReadyChannel() const;

    void readFromStream(std::shared_ptr<grpc::ClientReaderWriter<Request, Response>> &stream) const;

    std::shared_ptr<grpc::ClientReaderWriter<Request, Response>> &
    bidirectionalStream(std::shared_ptr<grpc::ClientReaderWriter<Request, Response>> &stream);
};

#endif
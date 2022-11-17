#ifndef ASRTEST_SPEECHCENTERCLIENT_H
#define ASRTEST_SPEECHCENTERCLIENT_H

#include "Configuration.h"

#include <grpcpp/security/credentials.h>
#include <grpcpp/impl/codegen/client_context.h>
#include "recognition.grpc.pb.h"
#include "recognition.pb.h"

class Audio;

class SpeechCenterClient {
public:
    SpeechCenterClient();
    ~SpeechCenterClient();

    void run(const Configuration& configuration);

private:
    std::shared_ptr<grpc::Channel> channel;
    grpc::ClientContext context;
    std::unique_ptr<speechcenter::recognizer::v1::RecognitionConfig> configMessage;
    std::unique_ptr<speechcenter::recognizer::v1::Recognizer::Stub> recognizer;
    std::unique_ptr<grpc::ClientReaderWriter<speechcenter::recognizer::v1::RecognitionStreamingRequest,
            speechcenter::recognizer::v1::RecognitionStreamingResponse>> stream;
    speechcenter::recognizer::v1::RecognitionStreamingResponse response;

    void connect(const Configuration& configuration);
    void process(const Configuration &configuration);


    static speechcenter::recognizer::v1::RecognitionResource_Model convertTopicModel(const std::string &modelName);

    // static std::string loadGrammarContent(const std::string &grammarPath);

    static std::unique_ptr<speechcenter::recognizer::v1::RecognitionResource>
    buildRecognitionResource(const Configuration &configuration);

    static std::unique_ptr<speechcenter::recognizer::v1::RecognitionParameters>
    buildRecognitionParameters(const Configuration &configuration);

    static std::unique_ptr<speechcenter::recognizer::v1::PCM>
    buildPCM(const std::string &sampleRate);

    static std::string readFileContent(const std::string &path);
    static std::string sanitize(std::string str);

    std::shared_ptr<grpc::Channel>
    createChannel(const Configuration &configuration);

    void createRecognizer();

    void sendAudio(Audio &audio);
};



#endif
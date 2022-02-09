#ifndef ASRTEST_SPEECHCENTERCLIENT_H
#define ASRTEST_SPEECHCENTERCLIENT_H

#include "Configuration.h"

#include <grpcpp/security/credentials.h>
#include <grpcpp/impl/codegen/client_context.h>
#include "verbio-speech-center.grpc.pb.h"
#include "verbio-speech-center.pb.h"


class SpeechCenterClient {
public:
    SpeechCenterClient();
    ~SpeechCenterClient();

    void connect(const Configuration& configuration);
    void process(const std::string &audioPath);

private:
    std::shared_ptr<grpc::Channel> channel;
    grpc::ClientContext context;
    std::unique_ptr<csr_grpc_gateway::RecognitionInit> initMessage;
    std::unique_ptr<csr_grpc_gateway::SpeechRecognizer::Stub> recognizer;
    std::unique_ptr<grpc::ClientWriter<csr_grpc_gateway::RecognitionRequest>> stream;
    csr_grpc_gateway::RecognitionResponse response;

    static csr_grpc_gateway::RecognitionResource_Model convertTopicModel(const std::string &modelName);

    static std::string loadGrammarContent(const std::string &grammarPath);

    static std::unique_ptr<csr_grpc_gateway::RecognitionResource>
    buildRecognitionResource(const Configuration &configuration);

    static std::unique_ptr<csr_grpc_gateway::RecognitionParameters>
    buildRecognitionParameters(const Configuration &configuration);

    static std::string readFileContent(const std::string &path);

    static grpc::SslCredentialsOptions buildSslCredentialOptions(const Configuration &configuration);

    std::shared_ptr<grpc::Channel>
    createChannel(const Configuration &configuration);
};



#endif
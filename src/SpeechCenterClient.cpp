#include "SpeechCenterClient.h"

#include "gRpcExceptions.h"
#include "Configuration.h"
#include "sndfile.hh"

#include "logger.h"

#include <grpcpp/create_channel.h>

#include <thread>
#include <sstream>
#include <fstream>


class Audio {
public:
    explicit Audio(const std::string &audioPath);
    ~Audio();
    const int16_t* getData() const {return data;}
    int16_t* getData() {return data;}
    int64_t getLengthInFrames() const {return length;}
    int64_t getLengthInBytes() const {return length*getBytesPerSamples();}

private:
    int16_t *data{nullptr};
    int64_t length{0};

    int64_t getBytesPerSamples() const {return sizeof(*data);}
};


Audio::Audio(const std::string &audioPath) {
    SndfileHandle sndfileHandle(audioPath);
    if (sndfileHandle.channels() != 1)
        throw GrpcException("Audio file must be mono");
    if (sndfileHandle.samplerate() != 8000)
        throw GrpcException("Audio file must be 8 kHz.");

    length = sndfileHandle.frames();
    data = new int16_t[length];
    length = sndfileHandle.read(data, length);
    INFO("Read {} samples with {} bytes per sample", sndfileHandle.frames(), getBytesPerSamples());
}

Audio::~Audio() {
    delete data;
}


SpeechCenterClient::SpeechCenterClient() = default;
SpeechCenterClient::~SpeechCenterClient() = default;


void SpeechCenterClient::connect(const Configuration& configuration) {
    createChannel(configuration);

    INFO("State: {}", toascii(channel->GetState(true)));
    INFO("Config JSON: {}", channel->GetServiceConfigJSON());

    recognizer = csr_grpc_gateway::SpeechRecognizer::NewStub(channel);

    INFO( "recognizer created");
    std::this_thread::sleep_for(std::chrono::seconds(3));

    stream = recognizer->RecognizeStream(&context, &response);

    INFO("R: {} ",  response.DebugString());
    INFO("Stream created");
    INFO("State: {} ",  toascii(channel->GetState(true)));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    INFO("State: {} ",  toascii(channel->GetState(true)));
//    INFO("Metadata:");
//    stream->WaitForInitialMetadata();
//
//    for (const auto &data : context.GetServerTrailingMetadata())
//        std::cout << data.first << " = " << data.second;
//
//    for (const auto &data : context.GetServerInitialMetadata())
//        std::cout << data.first << " = " <<  data.second;

    initMessage = std::make_unique<csr_grpc_gateway::RecognitionInit>();
    initMessage->set_allocated_resource(buildRecognitionResource(configuration).release());
    initMessage->set_allocated_parameters(buildRecognitionParameters(configuration).release());
    std::this_thread::sleep_for(std::chrono::seconds(3));
}




void SpeechCenterClient::process(const std::string &audioPath) {

    Audio audio(audioPath);

    csr_grpc_gateway::RecognitionRequest initRequest;

    initRequest.set_allocated_init(new csr_grpc_gateway::RecognitionInit(*initMessage));
    if (stream) {
        INFO("Sending: \n{} ",  initRequest.DebugString());
        if (!stream->Write(initRequest)) {
            auto status = stream->Finish();
            ERROR("{} ({}: {})",  status.error_message(), status.error_code(), status.error_details());
        } else {
            csr_grpc_gateway::RecognitionRequest audioRequest;
            audioRequest.set_audio(static_cast<void*>(audio.getData()), audio.getLengthInBytes());
            if (!stream->Write(audioRequest)) {
                auto status = stream->Finish();
                ERROR("{} ({}: {})", status.error_message(), status.error_code(), status.error_details());
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));

        stream->WritesDone();
        stream.reset();
    }
    INFO("Response: {}", response.DebugString());
    recognizer.reset();

}


std::shared_ptr<grpc::Channel>
SpeechCenterClient::createChannel(const Configuration &configuration) {
    auto jwt = readFileContent(configuration.getTokenPath());
    std::cout << "TOKEN:" << jwt << std::endl;
    channel = grpc::CreateChannel(configuration.getHost(),
                                  grpc::CompositeChannelCredentials(
                                          grpc::SslCredentials(grpc::SslCredentialsOptions()),
                                          grpc::AccessTokenCredentials(jwt)
                                  )
    );
    context.set_credentials(grpc::AccessTokenCredentials(jwt));
    channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(5));
    if (channel->GetState(false) != GRPC_CHANNEL_READY)
        if (!channel->WaitForStateChange(GRPC_CHANNEL_READY,
                                         std::chrono::system_clock::now() + std::chrono::seconds(5))) {
            WARN("Channel could get ready.");
            return channel;
        }
    INFO("Channel is ready");
    return channel;
}

std::unique_ptr<csr_grpc_gateway::RecognitionParameters>
SpeechCenterClient::buildRecognitionParameters(const Configuration &configuration) {
    std::unique_ptr<csr_grpc_gateway::RecognitionParameters> parameters (new csr_grpc_gateway::RecognitionParameters());
    parameters->set_language(configuration.getLanguage());
    return parameters;
}

std::unique_ptr<csr_grpc_gateway::RecognitionResource>
SpeechCenterClient::buildRecognitionResource(const Configuration &configuration) {
    std::unique_ptr<csr_grpc_gateway::RecognitionResource> resource (new csr_grpc_gateway::RecognitionResource());
    if (configuration.getGrammarPath().empty())
        resource->set_model(convertTopicModel(configuration.getTopic()));
    else
        resource->set_inline_grammar(loadGrammarContent(configuration.getGrammarPath()));
    return resource;
}

std::string SpeechCenterClient::loadGrammarContent(const std::string &grammarPath) {
    return readFileContent(grammarPath);
}

csr_grpc_gateway::RecognitionResource_Model SpeechCenterClient::convertTopicModel(const std::string &modelName) {
    csr_grpc_gateway::RecognitionResource_Model model;
    if (modelName == "generic")
        model = csr_grpc_gateway::RecognitionResource_Model_GENERIC;
    else if (modelName == "banking")
        model = csr_grpc_gateway::RecognitionResource_Model_BANKING;
    else if (modelName == "telco")
        model = csr_grpc_gateway::RecognitionResource_Model_TELCO;
    else
        throw UnknownTopicModel(modelName);
    return model;
}


std::string SpeechCenterClient::readFileContent(const std::string &path) {
    std::ifstream ifs(path);
    std::string content;
    if (ifs) {
        std::ostringstream oss;
        oss << ifs.rdbuf();
        content = oss.str();
    } else {
        throw IOError("Unable to open '" + path + "'");
    }
    return content;
}



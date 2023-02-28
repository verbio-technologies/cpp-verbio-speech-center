#include "RecognitionClient.h"

#include "gRpcExceptions.h"
#include "Configuration.h"
#include "Audio.h"

#include "logger.h"

#include <grpcpp/create_channel.h>

#include <thread>
#include <sstream>

const std::string uppercaseString(const std::string &str) {
    std::locale loc;
    std::string upper = "";
    for (std::string::size_type i=0; i<str.length(); ++i)
        upper += std::toupper(str[i],loc);
    return upper;
}

RecognitionClient::RecognitionClient(const Configuration &configuration) {
    channel = createChannel(configuration);

    stub_ = speechcenter::recognizer::v1::Recognizer::NewStub(channel);
};

RecognitionClient::~RecognitionClient() = default;

std::shared_ptr<grpc::Channel>
RecognitionClient::createChannel(const Configuration &configuration) {
    auto jwt = readFileContent(configuration.getTokenPath());

    // GRPC Non/secure toggle
    if(configuration.getNotSecure()) { // Not secure:
        WARN("Establishing insecure connection.");
        channel = grpc::CreateChannel(configuration.getHost(),
                                      grpc::InsecureChannelCredentials()
        );
        context.AddMetadata("authorization", "Bearer" + jwt);
    }
    else { // Secure
        channel = grpc::CreateChannel(configuration.getHost(),
                                      grpc::CompositeChannelCredentials(
                                      grpc::SslCredentials(grpc::SslCredentialsOptions()),
                                      grpc::AccessTokenCredentials(jwt)
                                      )
        );
    }

    channel->WaitForConnected(std::chrono::system_clock::now() + std::chrono::seconds(5));
    if (channel->GetState(false) != GRPC_CHANNEL_READY)
        if (!channel->WaitForStateChange(GRPC_CHANNEL_READY,
                                         std::chrono::system_clock::now() + std::chrono::seconds(5))) {
            WARN("Channel could get ready.");
            return channel;
        }
    INFO("Channel is ready. State {}", toascii(channel->GetState(true)));
    INFO("Channel configuration: {}", channel->GetServiceConfigJSON());
    return channel;
}

void RecognitionClient::connect(const Configuration& configuration) {
    std::shared_ptr<grpc::ClientReaderWriter<speechcenter::recognizer::v1::RecognitionStreamingRequest,
            speechcenter::recognizer::v1::RecognitionStreamingResponse>> stream(
            stub_->StreamingRecognize(&context));

    INFO("Stream CREATED. State {}",  toascii(channel->GetState(true)));

    // Write

    std::thread writer([stream, &configuration]() {
        INFO("WRITE: STARTING...");
        speechcenter::recognizer::v1::RecognitionStreamingRequest recognitionConfig = buildRecognitionConfig(configuration);
        INFO("Sending config: \n{} ",  recognitionConfig.DebugString());
        bool streamFail = !stream->Write(recognitionConfig);
        if(streamFail) {
            auto status = stream->Finish();
            ERROR("{} ({}: {})",  status.error_message(), status.error_code(), status.error_details());
            return;
        }

        speechcenter::recognizer::v1::RecognitionStreamingRequest audioRequest = buildAudioRequest(configuration);
        INFO("Sending audio...");
        streamFail = !stream->Write(audioRequest);
        if(streamFail) {
            auto status = stream->Finish();
            ERROR("{} ({}: {})",  status.error_message(), status.error_code(), status.error_details());
            return;
        }

        stream->WritesDone();
        INFO("WRITE: ...FINISHED");
    });

    // Read

    INFO("READ: STARTING...");

    speechcenter::recognizer::v1::RecognitionStreamingResponse response;

    while(stream->Read(&response)) {
        if (response.result().is_final() && !response.result().alternatives().empty()) {
            speechcenter::recognizer::v1::RecognitionAlternative firstAlternative = response.result().alternatives().Get(0);
            if(firstAlternative.words_size() > 0){
                INFO("Segment start: {}", firstAlternative.words()[0].start_time().seconds());
                std::cout << firstAlternative.transcript() << std::endl;
                INFO("Segment end: {}", firstAlternative.words()[firstAlternative.words_size()-1].end_time().seconds());
            }
        }
        else {
            WARN("No recognition result alternatives!");
        }
    }
    writer.join();
    grpc::Status status = stream->Finish();
    if(!status.ok()) {
        ERROR("RESPONSE ERROR!\n\n");
    }
}

speechcenter::recognizer::v1::RecognitionStreamingRequest
RecognitionClient::buildRecognitionConfig(const Configuration& configuration) {
    std::unique_ptr<speechcenter::recognizer::v1::RecognitionConfig> configMessage;

    configMessage = std::make_unique<speechcenter::recognizer::v1::RecognitionConfig>();
    configMessage->set_allocated_resource(buildRecognitionResource(configuration).release());
    configMessage->set_allocated_parameters(buildRecognitionParameters(configuration).release());
    configMessage->set_version(buildAsrVersion(configuration));

    speechcenter::recognizer::v1::RecognitionStreamingRequest recognitionConfig;
    recognitionConfig.set_allocated_config(new speechcenter::recognizer::v1::RecognitionConfig(*configMessage));

    return recognitionConfig;
}

speechcenter::recognizer::v1::RecognitionStreamingRequest
RecognitionClient::buildAudioRequest(const Configuration& configuration) {
    INFO("Building audio request...");

    Audio audio(configuration.getAudioPath());

    int64_t lengthInBytes = audio.getLengthInBytes();

    INFO("Audio bytes: " + std::to_string(lengthInBytes));
    speechcenter::recognizer::v1::RecognitionStreamingRequest audioRequest;
    audioRequest.set_audio(static_cast<void*>(audio.getData()), lengthInBytes);

    return audioRequest;
}

std::unique_ptr<speechcenter::recognizer::v1::RecognitionParameters>
RecognitionClient::buildRecognitionParameters(const Configuration &configuration) {
    std::unique_ptr<speechcenter::recognizer::v1::RecognitionParameters> parameters (new speechcenter::recognizer::v1::RecognitionParameters());
    parameters->set_language(configuration.getLanguage());
    parameters->set_allocated_pcm(buildPCM(configuration.getSampleRate()).release());
    return parameters;
}

std::unique_ptr<speechcenter::recognizer::v1::PCM>
RecognitionClient::buildPCM(const uint32_t &sampleRate) {
    std::unique_ptr<speechcenter::recognizer::v1::PCM> pcm (new speechcenter::recognizer::v1::PCM());
    if(sampleRate != 16000 && sampleRate != 8000)
        throw UnsupportedSampleRate(std::to_string(sampleRate));
    pcm->set_sample_rate_hz(sampleRate);
    return pcm;
}

std::unique_ptr<speechcenter::recognizer::v1::RecognitionResource>
RecognitionClient::buildRecognitionResource(const Configuration &configuration) {
    std::unique_ptr<speechcenter::recognizer::v1::RecognitionResource> resource (new speechcenter::recognizer::v1::RecognitionResource());

    resource->set_topic(convertTopic(configuration.getTopic()));
    return resource;
}

speechcenter::recognizer::v1::RecognitionResource_Model
RecognitionClient::convertTopic(const std::string &topicName) {
    static const std::unordered_map<std::string, speechcenter::recognizer::v1::RecognitionResource_Model>
            validTopics = {
            {"GENERIC", speechcenter::recognizer::v1::RecognitionResource_Model_GENERIC},
            {"BANKING", speechcenter::recognizer::v1::RecognitionResource_Model_BANKING},
            {"TELCO", speechcenter::recognizer::v1::RecognitionResource_Model_TELCO},
            {"INSURANCE", speechcenter::recognizer::v1::RecognitionResource_Model_INSURANCE}
    };

    std::string topicUpper = uppercaseString(topicName);

    auto topicIter = validTopics.find(topicUpper);
    if(topicIter == validTopics.end()) {
        throw UnknownTopicModel(topicUpper);
    }

    return topicIter->second;
}

::speechcenter::recognizer::v1::RecognitionConfig_AsrVersion
RecognitionClient::buildAsrVersion(const Configuration &configuration) {
    const std::unordered_map<std::string, ::speechcenter::recognizer::v1::RecognitionConfig_AsrVersion>
            validAsrVersions = {
            {"V1", ::speechcenter::recognizer::v1::RecognitionConfig_AsrVersion::RecognitionConfig_AsrVersion_V1},
            {"V2", ::speechcenter::recognizer::v1::RecognitionConfig_AsrVersion::RecognitionConfig_AsrVersion_V2}
    };

    std::string asrVersion = uppercaseString(configuration.getAsrVersion());

    auto topicIter = validAsrVersions.find(asrVersion);
    if(topicIter == validAsrVersions.end()) {
        throw UnknownAsrVersion(asrVersion);
    }

    return topicIter->second;
}

std::string RecognitionClient::readFileContent(const std::string &path) {
    std::ifstream ifs(path);
    std::string content;
    if (ifs) {
        std::ostringstream oss;
        oss << ifs.rdbuf();
        content = oss.str();

    } else {
        throw IOError("Unable to open '" + path + "'");
    }

    return sanitize(content);
}

std::string RecognitionClient::sanitize(std::string str) {
    size_t endpos = str.find_last_not_of("\r\n");
    if(endpos != std::string::npos) {
        str.substr(0,endpos+1).swap(str);
    }
    return str;
}


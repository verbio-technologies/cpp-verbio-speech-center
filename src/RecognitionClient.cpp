#include "RecognitionClient.h"

#include "Audio.h"
#include "Configuration.h"
#include "gRpcExceptions.h"

#include "logger.h"

#include <grpcpp/create_channel.h>

#include <future>
#include <sstream>
#include <thread>

void RecognitionClient::write(
        std::shared_ptr<grpc::ClientReaderWriter<RecognitionStreamingRequest,
                                                 RecognitionStreamingResponse>>
                stream) {

    INFO("WRITE: STARTING...");
    speechcenter::recognizer::v1::RecognitionStreamingRequest recognitionConfig =
            buildRecognitionConfig();
    INFO("Sending config: \n{} ", recognitionConfig.DebugString());
    bool streamFail = !stream->Write(recognitionConfig);
    if (streamFail) {
        auto status = stream->Finish();
        ERROR("{} ({}: {})", status.error_message(), status.error_code(),
              status.error_details());
        throw StreamException(status.error_message());
    }

    speechcenter::recognizer::v1::RecognitionStreamingRequest audioRequest =
            buildAudioRequest();
    INFO("Sending audio...");
    streamFail = !stream->Write(audioRequest);
    if (streamFail) {
        auto status = stream->Finish();
        ERROR("{} ({}: {})", status.error_message(), status.error_code(),
              status.error_details());
        throw StreamException(status.error_message());
    }

    stream->WritesDone();
    INFO("Audio sent");
}

std::string uppercaseString(const std::string &str) {
    std::locale loc;
    std::string upper = "";
    for (std::string::size_type i = 0; i < str.length(); ++i)
        upper += std::toupper(str[i], loc);
    return upper;
}

RecognitionClient::RecognitionClient(const Configuration &configuration) {
    INFO("Started recognition session...");
    this->configuration = configuration;
    channel = createChannel();
    stub_ = speechcenter::recognizer::v1::Recognizer::NewStub(channel);
};

RecognitionClient::~RecognitionClient() = default;

void RecognitionClient::test(int i) { sleep(10); };

std::shared_ptr<grpc::Channel> RecognitionClient::createChannel() {
    auto jwt = readFileContent(configuration.getTokenPath());

    // GRPC Non/secure toggle
    if (configuration.getNotSecure()) {// Not secure:
        WARN("Establishing insecure connection.");
        channel = grpc::CreateChannel(configuration.getHost(),
                                      grpc::InsecureChannelCredentials());
        context.AddMetadata("authorization", "Bearer" + jwt);
    } else {// Secure
        channel = grpc::CreateChannel(
                configuration.getHost(),
                grpc::CompositeChannelCredentials(
                        grpc::SslCredentials(grpc::SslCredentialsOptions()),
                        grpc::AccessTokenCredentials(jwt)));
    }

    channel->WaitForConnected(std::chrono::system_clock::now() +
                              std::chrono::seconds(5));
    if (channel->GetState(false) != GRPC_CHANNEL_READY)
        if (!channel->WaitForStateChange(GRPC_CHANNEL_READY,
                                         std::chrono::system_clock::now() +
                                                 std::chrono::seconds(5))) {
            WARN("Channel could get ready.");
            return channel;
        }
    INFO("Channel is ready. State {}", toascii(channel->GetState(true)));
    INFO("Channel configuration: {}", channel->GetServiceConfigJSON());
    return channel;
}

void RecognitionClient::connect(const Configuration &configuration) {
    std::shared_ptr<grpc::ClientReaderWriter<
            speechcenter::recognizer::v1::RecognitionStreamingRequest,
            speechcenter::recognizer::v1::RecognitionStreamingResponse>>
            stream(stub_->StreamingRecognize(&context));

    INFO("Stream CREATED. State {}", toascii(channel->GetState(true)));

    // Write
    std::packaged_task<void(
            std::shared_ptr<grpc::ClientReaderWriter<
                    speechcenter::recognizer::v1::RecognitionStreamingRequest,
                    speechcenter::recognizer::v1::RecognitionStreamingResponse>>)>
            parallel_write(
                    [this](
                            std::shared_ptr<grpc::ClientReaderWriter<
                                    speechcenter::recognizer::v1::RecognitionStreamingRequest,
                                    speechcenter::recognizer::v1::RecognitionStreamingResponse>>
                                    stream) {
                        write(stream);
                    });
    auto result = parallel_write.get_future();
    auto thread = std::thread{std::move(parallel_write), stream};

    // Read

    INFO("READ: STARTING...");

    speechcenter::recognizer::v1::RecognitionStreamingResponse response;

    while (stream->Read(&response)) {
        if (response.result().is_final() &&
            !response.result().alternatives().empty()) {
            speechcenter::recognizer::v1::RecognitionAlternative firstAlternative =
                    response.result().alternatives().Get(0);
            if (firstAlternative.words_size() > 0) {
                INFO("Segment start: {}", firstAlternative.words()[0].start_time());
                std::cout << firstAlternative.transcript() << std::endl;
                INFO("Segment end: {}",
                     firstAlternative.words()[firstAlternative.words_size() - 1]
                             .end_time());
            }
        } else {
            WARN("No recognition result alternatives!");
        }
    }
    thread.join();
    result.get();
    grpc::Status status = stream->Finish();
    if (!status.ok()) {
        ERROR("RESPONSE ERROR!\n\n");
        throw StreamException(status.error_message());
    }
}

speechcenter::recognizer::v1::RecognitionStreamingRequest
RecognitionClient::buildRecognitionConfig() {
    std::unique_ptr<speechcenter::recognizer::v1::RecognitionConfig>
            configMessage;

    configMessage =
            std::make_unique<speechcenter::recognizer::v1::RecognitionConfig>();
    configMessage->set_allocated_resource(buildRecognitionResource().release());
    configMessage->set_allocated_parameters(
            buildRecognitionParameters().release());
    configMessage->set_version(buildAsrVersion());

    speechcenter::recognizer::v1::RecognitionStreamingRequest recognitionConfig;
    recognitionConfig.set_allocated_config(
            new speechcenter::recognizer::v1::RecognitionConfig(*configMessage));

    return recognitionConfig;
}

speechcenter::recognizer::v1::RecognitionStreamingRequest
RecognitionClient::buildAudioRequest() {
    INFO("Building audio request...");

    Audio audio(configuration.getAudioPath());

    int64_t lengthInBytes = audio.getLengthInBytes();

    INFO("Audio bytes: " + std::to_string(lengthInBytes));
    speechcenter::recognizer::v1::RecognitionStreamingRequest audioRequest;
    audioRequest.set_audio(static_cast<void *>(audio.getData()), lengthInBytes);

    return audioRequest;
}

std::unique_ptr<RecognitionParameters>
RecognitionClient::buildRecognitionParameters() {
    std::unique_ptr<speechcenter::recognizer::v1::RecognitionParameters>
            parameters(new speechcenter::recognizer::v1::RecognitionParameters());
    parameters->set_language(configuration.getLanguage());
    parameters->set_allocated_pcm(
            buildPCM(configuration.getSampleRate()).release());
    INFO("Enabled formatting: {}", configuration.getFormatting());
    parameters->set_enable_formatting(configuration.getFormatting());
    INFO("Enabled diarization: {}", configuration.getDiarization());
    parameters->set_enable_diarization(configuration.getDiarization());

    return parameters;
}

std::unique_ptr<speechcenter::recognizer::v1::PCM>
RecognitionClient::buildPCM(const uint32_t &sampleRate) {
    std::unique_ptr<speechcenter::recognizer::v1::PCM> pcm(
            new speechcenter::recognizer::v1::PCM());
    if (sampleRate != 16000 && sampleRate != 8000)
        throw UnsupportedSampleRate(std::to_string(sampleRate));
    pcm->set_sample_rate_hz(sampleRate);
    return pcm;
}

std::unique_ptr<RecognitionResource>
RecognitionClient::buildRecognitionResource() {
    std::unique_ptr<speechcenter::recognizer::v1::RecognitionResource> resource(
            new speechcenter::recognizer::v1::RecognitionResource());

    resource->set_topic(convertTopic(configuration.getTopic()));
    return resource;
}

speechcenter::recognizer::v1::RecognitionResource_Topic
RecognitionClient::convertTopic(const std::string &topicName) {
    static const std::unordered_map<
            std::string, speechcenter::recognizer::v1::RecognitionResource_Topic>
            validTopics = {
                    {"GENERIC",
                     speechcenter::recognizer::v1::RecognitionResource_Topic_GENERIC},
                    {"BANKING",
                     speechcenter::recognizer::v1::RecognitionResource_Topic_BANKING},
                    {"TELCO",
                     speechcenter::recognizer::v1::RecognitionResource_Topic_TELCO},
                    {"INSURANCE",
                     speechcenter::recognizer::v1::RecognitionResource_Topic_INSURANCE}};

    std::string topicUpper = uppercaseString(topicName);

    auto topicIter = validTopics.find(topicUpper);
    if (topicIter == validTopics.end()) {
        throw UnknownTopicModel(topicUpper);
    }

    return topicIter->second;
}

RecognitionConfig_AsrVersion RecognitionClient::buildAsrVersion() {
    static const std::unordered_map<
            std::string, ::speechcenter::recognizer::v1::RecognitionConfig_AsrVersion>
            validAsrVersions = {
                    {"V1", ::speechcenter::recognizer::v1::RecognitionConfig_AsrVersion::
                                   RecognitionConfig_AsrVersion_V1},
                    {"V2", ::speechcenter::recognizer::v1::RecognitionConfig_AsrVersion::
                                   RecognitionConfig_AsrVersion_V2}};

    std::string asrVersion = uppercaseString(configuration.getAsrVersion());

    auto topicIter = validAsrVersions.find(asrVersion);
    if (topicIter == validAsrVersions.end()) {
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
    if (endpos != std::string::npos) {
        str.substr(0, endpos + 1).swap(str);
    }
    return str;
}

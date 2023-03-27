#include "RecognitionClient.h"

#include "Audio.h"
#include "Configuration.h"
#include "gRpcExceptions.h"

#include "logger.h"

#include <grpcpp/create_channel.h>

#include <chrono>
#include <future>
#include <sstream>
#include <thread>

using namespace speechcenter::recognizer::v1;
typedef RecognitionStreamingRequest Request;
typedef RecognitionStreamingResponse Response;

void RecognitionClient::write(
        std::shared_ptr<grpc::ClientReaderWriter<Request,
                Response>>
        stream) {

    INFO("WRITE: STARTING...");
    Request recognitionConfig = buildRecognitionConfig();
    INFO("Sending config: \n{} ", recognitionConfig.DebugString());
    bool streamFail = !stream->Write(recognitionConfig);
    if (streamFail) {
        auto status = stream->Finish();
        ERROR("{} ({}: {})", status.error_message(), status.error_code(), status.error_details());
        throw StreamException(status.error_message());
    }

    INFO("Sending audio...");
    int requestCount = 0;
    double totalAudioSentInSeconds{0};
    for (const auto &request: buildAudioRequests()) {
        constexpr int bytesPerSamples = 2;// PCM16
        double loopAudioSentInMilliseconds = request.audio().length() * 1000.00 / (bytesPerSamples * configuration.getSampleRate());
        auto deadline = std::chrono::system_clock::now() +
                        std::chrono::milliseconds(static_cast<int64_t>(loopAudioSentInMilliseconds));
        if (!stream->Write(request)) {
            auto status = stream->Finish();
            ERROR("{} ({}: {})", status.error_message(), status.error_code(), status.error_details());
            throw StreamException(status.error_message());
        }
        ++requestCount;
        totalAudioSentInSeconds += loopAudioSentInMilliseconds / 1000.00;

        DEBUG ("Sent {} bytes of audio", requestCount * request.audio().length());
        DEBUG("Total time of audio sent {}", totalAudioSentInSeconds); 

        std::this_thread::sleep_until(deadline);
    }
    stream->WritesDone();
    INFO("All audio sent in {} requests.", requestCount);
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
    stub_ = Recognizer::NewStub(channel);
};

RecognitionClient::~RecognitionClient() = default;

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
    std::shared_ptr<grpc::ClientReaderWriter<Request, Response>> stream(stub_->StreamingRecognize(&context));

    INFO("Stream CREATED. State {}", toascii(channel->GetState(true)));

    // Write
    std::packaged_task<void(
            std::shared_ptr<grpc::ClientReaderWriter<Request, Response>>)>
            parallel_write(
            [this](
                    std::shared_ptr<grpc::ClientReaderWriter<Request, Response>> stream) { write(stream); });
    auto result = parallel_write.get_future();

    auto thread = std::thread{std::move(parallel_write), stream};

    // Read

    INFO("READ: STARTING...");

    Response response;

    while (stream->Read(&response)) {
        if (response.result().is_final() &&
            !response.result().alternatives().empty()) {
            RecognitionAlternative firstAlternative =
                    response.result().alternatives().Get(0);
            if (firstAlternative.words_size() > 0) {
                INFO("Segment start: {}", firstAlternative.words()[0].start_time());
                std::cout << firstAlternative.transcript() << std::endl;
                INFO("Segment end: {}",
                     firstAlternative.words()[firstAlternative.words_size() - 1].end_time());
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

Request
RecognitionClient::buildRecognitionConfig() {
    std::unique_ptr<RecognitionConfig>
            configMessage;

    configMessage =
            std::make_unique<RecognitionConfig>();
    configMessage->set_allocated_resource(buildRecognitionResource().release());
    configMessage->set_allocated_parameters(
            buildRecognitionParameters().release());
    configMessage->set_version(buildAsrVersion());

    Request recognitionConfig;
    recognitionConfig.set_allocated_config(
            new RecognitionConfig(*configMessage));

    return recognitionConfig;
}

std::vector<Request> RecognitionClient::buildAudioRequests() {
    INFO("Building audio request...");
    std::string path = configuration.getAudioPath();
    auto const &audio = Audio(path);
    int64_t lengthInBytes = audio.getLengthInBytes();
    INFO("Audio bytes: " + std::to_string(lengthInBytes));

    std::vector<Request> requests;
    auto chunks = audio.getAudioChunks(configuration.getChunkSize());
    for (const auto &chunk: chunks) {
        Request request;
        request.set_audio(static_cast<const void *>(chunk.data()), chunk.size() * audio.getBytesPerSamples());
        requests.emplace_back(request);
    }
    return requests;
}

std::unique_ptr<RecognitionParameters>
RecognitionClient::buildRecognitionParameters() {
    std::unique_ptr<RecognitionParameters>
            parameters(new RecognitionParameters());
    parameters->set_language(configuration.getLanguage());
    parameters->set_allocated_pcm(
            buildPCM(configuration.getSampleRate()).release());
    INFO("Enabled formatting: {}", configuration.getFormatting());
    parameters->set_enable_formatting(configuration.getFormatting());
    INFO("Enabled diarization: {}", configuration.getDiarization());
    parameters->set_enable_diarization(configuration.getDiarization());
    parameters->set_audio_channels_number(configuration.getNumberOfChannels());

    return parameters;
}

std::unique_ptr<PCM>
RecognitionClient::buildPCM(const uint32_t &sampleRate) {
    std::unique_ptr<PCM> pcm(
            new PCM());
    pcm->set_sample_rate_hz(sampleRate);
    return pcm;
}

std::unique_ptr<RecognitionResource>
RecognitionClient::buildRecognitionResource() {
    std::unique_ptr<RecognitionResource> resource(
            new RecognitionResource());

    resource->set_topic(convertTopic(configuration.getTopic()));
    return resource;
}

RecognitionResource_Topic
RecognitionClient::convertTopic(const std::string &topicName) {
    static const std::unordered_map<
            std::string, RecognitionResource_Topic>
            validTopics = {
            {"GENERIC",
                    RecognitionResource_Topic_GENERIC},
            {"BANKING",
                    RecognitionResource_Topic_BANKING},
            {"TELCO",
                    RecognitionResource_Topic_TELCO},
            {"INSURANCE",
                    RecognitionResource_Topic_INSURANCE}};

    std::string topicUpper = uppercaseString(topicName);

    auto topicIter = validTopics.find(topicUpper);
    if (topicIter == validTopics.end()) {
        throw UnknownTopicModel(topicUpper);
    }

    return topicIter->second;
}

RecognitionConfig_AsrVersion RecognitionClient::buildAsrVersion() {
    static const std::unordered_map<
            std::string, ::RecognitionConfig_AsrVersion>
            validAsrVersions = {
            {"V1", ::RecognitionConfig_AsrVersion::
                   RecognitionConfig_AsrVersion_V1},
            {"V2", ::RecognitionConfig_AsrVersion::
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

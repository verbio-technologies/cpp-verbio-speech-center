#include "RecognitionClient.h"

#include "Audio.h"
#include "Configuration.h"
#include "gRpcExceptions.h"

#include "logger.h"
#include "SpeechCenterCredentials.h"

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

    INFO("Writing to stream...");
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
    for (const auto &request: buildAudioRequests()) {
        constexpr int bytesPerSamples = 2;// PCM16
        auto deadline = std::chrono::system_clock::now() +
                        std::chrono::milliseconds(
                                request.audio().length() * 1000 / (bytesPerSamples * configuration.getSampleRate()));
        if (!stream->Write(request)) {
            auto status = stream->Finish();
            ERROR("{} (GRPC_ERR_CODE {} - {})", status.error_message(), status.error_code(), status.error_details());
            throw StreamException(status.error_message());
        }
        ++requestCount;
        if (requestCount % 10 == 0)
            INFO("Sent {} bytes of audio", requestCount * request.audio().length());

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
    std::string jwt = getJwtToken();
    establishConnection(jwt);
    return getReadyChannel();
}

std::shared_ptr<grpc::Channel> RecognitionClient::getReadyChannel() const {
    channel->WaitForConnected(std::chrono::system_clock::now() +
                              std::chrono::seconds(5));
    if (channel->GetState(false) != GRPC_CHANNEL_READY)
        if (!channel->WaitForStateChange(GRPC_CHANNEL_READY,
                                         std::chrono::system_clock::now() +
                                         std::chrono::seconds(5))) {
            WARN("Channel could not get ready.");
            return channel;
        }
    INFO("Channel is READY.");
    INFO("Channel configuration: '{}'", channel->GetServiceConfigJSON());
    return channel;
}

void RecognitionClient::establishConnection(const std::string &jwt) {
    if (configuration.getNotSecure()) {
        WARN("Establishing insecure connection.");
        channel = grpc::CreateChannel(configuration.getHost(),
                                      grpc::InsecureChannelCredentials());
        context.AddMetadata("authorization", "Bearer" + jwt);
    } else {
        INFO( "Establishing secure connection.");
        channel = grpc::CreateChannel(
                configuration.getHost(),
                grpc::CompositeChannelCredentials(
                        grpc::SslCredentials(grpc::SslCredentialsOptions()),
                        grpc::AccessTokenCredentials(jwt)));
    }
}

std::string RecognitionClient::getJwtToken() const {
    auto speechCenterCredentials = SpeechCenterCredentials(configuration.getTokenPath());
    if (!configuration.getClientId().empty() && !configuration.getClientSecret().empty()) {
        INFO("Automatic token refresh enabled. Writing new tokens to file: '{}'", configuration.getTokenPath());
        speechCenterCredentials.setClientCredentials(configuration.getClientId(), configuration.getClientSecret());
    }
    return speechCenterCredentials.getToken();
}

void RecognitionClient::performStreamingRecognition() {
    std::shared_ptr<grpc::ClientReaderWriter<Request, Response>> stream(stub_->StreamingRecognize(&context));
    INFO("Stream created. State {}", channel->GetState(true));

    stream = bidirectionalStream(stream);

    grpc::Status status = stream->Finish();
    if (!status.ok()) {
        ERROR("RESPONSE ERROR!\n\n");
        throw StreamException(status.error_message());
    }
}

std::shared_ptr<grpc::ClientReaderWriter<Request, Response>> &
RecognitionClient::bidirectionalStream(std::shared_ptr<grpc::ClientReaderWriter<Request, Response>> &stream) {
    std::packaged_task<void(
            std::shared_ptr<grpc::ClientReaderWriter<Request, Response>>)>
            parallel_write(
            [this](
                    std::shared_ptr<grpc::ClientReaderWriter<Request, Response>> stream) { write(stream); });
    auto result = parallel_write.get_future();
    auto thread = std::thread{std::move(parallel_write), stream};

    readFromStream(stream);

    thread.join();
    result.get();
    return stream;
}

void RecognitionClient::readFromStream(std::shared_ptr<grpc::ClientReaderWriter<Request, Response>> &stream) const {

    INFO("Reading from stream...");
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
}

Request RecognitionClient::buildRecognitionConfig() {
    std::unique_ptr<RecognitionConfig>
            configMessage;

    configMessage =
            std::make_unique<RecognitionConfig>();
    configMessage->set_allocated_resource(buildRecognitionResource().release());
    configMessage->set_allocated_parameters(
            buildRecognitionParameters().release());
    configMessage->set_version(buildAsrVersion());
    configMessage->add_label(configuration.getLabel());

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
    auto chunks = audio.getAudioChunks<20000>();
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

    return parameters;
}

std::unique_ptr<PCM>
RecognitionClient::buildPCM(const uint32_t &sampleRate) {
    std::unique_ptr<PCM> pcm(
            new PCM());
    if (sampleRate != 16000 && sampleRate != 8000)
        throw UnsupportedSampleRate(std::to_string(sampleRate));
    pcm->set_sample_rate_hz(sampleRate);
    return pcm;
}

std::unique_ptr<RecognitionResource>
RecognitionClient::buildRecognitionResource() {
    std::unique_ptr<RecognitionResource> resource(
            new RecognitionResource());

    if (configuration.hasTopic())
        resource->set_topic(convertTopic(configuration.getTopic()));
    else if (configuration.hasGrammar())
        resource->set_allocated_grammar(buildGrammarResource(configuration.getGrammar()));
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
        ERROR("Unsupported topic: {}", topicName);
        throw UnknownTopicModel(topicUpper);
    }

    return topicIter->second;
}

GrammarResource*
RecognitionClient::buildGrammarResource(const Grammar &grammar) {
    GrammarResource* resource = new GrammarResource();
    std::vector<char> bytes;

    switch (grammar.getType()) {
        case GrammarType::INLINE:
            resource->set_inline_grammar(grammar.getContent());
            break;
        case GrammarType::URI:
            resource->set_grammar_uri(grammar.getContent());
            break;
        case GrammarType::COMPILED:
            bytes = grammar.getCompiledBytes();
            resource->set_compiled_grammar(bytes.data(), bytes.size());
            break;
        default:
            ERROR("Unsupported grammar: {}", grammar.getContent());
            throw UnknownGrammarModel(grammar.getContent());
    }

    return resource;
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


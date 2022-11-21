#include "RecognitionClient.h"

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

RecognitionClient::RecognitionClient(const Configuration &configuration) {
    channel = createChannel(configuration);

    stub_ = speechcenter::recognizer::v1::Recognizer::NewStub(channel);
};

RecognitionClient::~RecognitionClient() = default;

std::shared_ptr<grpc::Channel>
RecognitionClient::createChannel(const Configuration &configuration) {
    auto jwt = readFileContent(configuration.getTokenPath());
    std::cout << "TOKEN: -" << jwt << "-" << std::endl;
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
        if (!response.result().alternatives().empty()) {
            speechcenter::recognizer::v1::RecognitionAlternative firstAlternative = response.result().alternatives().Get(0);
            for(speechcenter::recognizer::v1::WordInfo word : firstAlternative.words() ) {
                INFO("{}", word.word());
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
RecognitionClient::buildPCM(const std::string &sampleRate) {
    std::unique_ptr<speechcenter::recognizer::v1::PCM> pcm (new speechcenter::recognizer::v1::PCM());
    uint32_t sampleRateHz;
    if(sampleRate == "16000" || sampleRate.empty())
        sampleRateHz = 16000;
    else
        throw UnsupportedSampleRate(sampleRate);
    pcm->set_sample_rate_hz(sampleRateHz);
    return pcm;
}

std::unique_ptr<speechcenter::recognizer::v1::RecognitionResource>
RecognitionClient::buildRecognitionResource(const Configuration &configuration) {
    std::unique_ptr<speechcenter::recognizer::v1::RecognitionResource> resource (new speechcenter::recognizer::v1::RecognitionResource());

    resource->set_topic(convertTopicModel(configuration.getTopic()));
    return resource;
}

speechcenter::recognizer::v1::RecognitionResource_Model RecognitionClient::convertTopicModel(const std::string &modelName) {
    speechcenter::recognizer::v1::RecognitionResource_Model model;
    if (modelName == "generic")
        model = speechcenter::recognizer::v1::RecognitionResource_Model_GENERIC;
    else if (modelName == "banking")
        model = speechcenter::recognizer::v1::RecognitionResource_Model_BANKING;
    else if (modelName == "telco")
        model = speechcenter::recognizer::v1::RecognitionResource_Model_TELCO;
    else if (modelName == "insurance")
        model = speechcenter::recognizer::v1::RecognitionResource_Model_INSURANCE;
    else
        throw UnknownTopicModel(modelName);
    return model;
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

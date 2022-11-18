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
    createRecognizer();

    stream = recognizer->StreamingRecognize(&context);
    INFO("Stream created. State {}",  toascii(channel->GetState(true)));
}

void SpeechCenterClient::process(const Configuration &configuration) {
    std::unique_ptr<speechcenter::recognizer::v1::RecognitionConfig> configMessage;

    configMessage = std::make_unique<speechcenter::recognizer::v1::RecognitionConfig>();
    configMessage->set_allocated_resource(buildRecognitionResource(configuration).release());
    configMessage->set_allocated_parameters(buildRecognitionParameters(configuration).release());

    INFO("Running SpeechCenterClient::process!");

    Audio audio(configuration.getAudioPath());

    speechcenter::recognizer::v1::RecognitionStreamingRequest recognitionConfig;

    recognitionConfig.set_allocated_config(new speechcenter::recognizer::v1::RecognitionConfig(*configMessage));
    if (stream) {
        INFO("Sending: \n{} ",  recognitionConfig.DebugString());
        if (!stream->Write(recognitionConfig)) {
            auto status = stream->Finish();
            ERROR("{} ({}: {})",  status.error_message(), status.error_code(), status.error_details());
        } else {
            sendAudio(audio);
        }

        stream->WritesDone();

        speechcenter::recognizer::v1::RecognitionStreamingResponse response;
        std::string resultText;

        uint64_t i;

        while(stream->Read(&response)) {
            //INFO("Processing response {}...", std::to_string(i));

            if (!response.result().alternatives().empty()) {
                speechcenter::recognizer::v1::RecognitionAlternative firstAlternative = response.result().alternatives().Get(0);
                for(speechcenter::recognizer::v1::WordInfo word : firstAlternative.words() ) {
                    resultText.append(word.word() + " ");
                }
            }
            else {
                //WARN("No recognition result alternatives!");
            }

            ++i;
        }

        grpc::Status status = stream->Finish();
        if(status.ok()) {
            INFO("RESPONSE:\n{}\n\n",  resultText);
        }
        else {
            ERROR("RESPONSE ERROR!\n\n{}\n\n",  resultText);
        }

        stream.reset();
    }
    recognizer.reset();
}

void SpeechCenterClient::sendAudio(Audio &audio) {
    int64_t lengthInBytes = audio.getLengthInBytes();

    INFO("Audio bytes: " + std::to_string(lengthInBytes));
    INFO("Audio request...");
    speechcenter::recognizer::v1::RecognitionStreamingRequest audioRequest;
    audioRequest.set_audio(static_cast<void*>(audio.getData()), lengthInBytes);
    if (!stream->Write(audioRequest)) {
        auto status = stream->Finish();
        ERROR("{} ({}: {})", status.error_message(), status.error_code(), status.error_details());
    }
    else {
        INFO("Sending audio...");
    }
}

void SpeechCenterClient::createRecognizer() {
    recognizer = speechcenter::recognizer::v1::Recognizer::NewStub(channel);
    INFO("Recognizer created");
}


std::shared_ptr<grpc::Channel>
SpeechCenterClient::createChannel(const Configuration &configuration) {
    auto jwt = readFileContent(configuration.getTokenPath());
    std::cout << "TOKEN: -" << jwt << "-" << std::endl;
    channel = grpc::CreateChannel(configuration.getHost(),
                                  grpc::InsecureChannelCredentials()
                                  /*grpc::CompositeChannelCredentials(
                                          grpc::SslCredentials(grpc::SslCredentialsOptions()),
                                          grpc::AccessTokenCredentials(jwt)
                                  )*/
    );
    //context.set_credentials(grpc::AccessTokenCredentials(jwt));
    context.AddMetadata("authorization",  "Bearer" + jwt); // TODO: delete
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

std::unique_ptr<speechcenter::recognizer::v1::RecognitionParameters>
SpeechCenterClient::buildRecognitionParameters(const Configuration &configuration) {
    std::unique_ptr<speechcenter::recognizer::v1::RecognitionParameters> parameters (new speechcenter::recognizer::v1::RecognitionParameters());
    parameters->set_language(configuration.getLanguage());
    parameters->set_allocated_pcm(buildPCM(configuration.getSampleRate()).release());
    return parameters;
}
std::unique_ptr<speechcenter::recognizer::v1::PCM>
SpeechCenterClient::buildPCM(const std::string &sampleRate) {
    std::unique_ptr<speechcenter::recognizer::v1::PCM> pcm (new speechcenter::recognizer::v1::PCM());
    uint32_t sampleRateHz;
    if(sampleRate == "16000" || sampleRate == "")
        sampleRateHz = 16000;
    else
        throw UnsupportedSampleRate(sampleRate);
    pcm->set_sample_rate_hz(sampleRateHz);
    return pcm;
}

std::unique_ptr<speechcenter::recognizer::v1::RecognitionResource>
SpeechCenterClient::buildRecognitionResource(const Configuration &configuration) {
    std::unique_ptr<speechcenter::recognizer::v1::RecognitionResource> resource (new speechcenter::recognizer::v1::RecognitionResource());
    /*
    if (configuration.getGrammarPath().empty())
        resource->set_topic(convertTopicModel(configuration.getTopic()));
    else
        resource->set_inline_grammar(loadGrammarContent(configuration.getGrammarPath()));
    */
    resource->set_topic(convertTopicModel(configuration.getTopic()));
    return resource;
}

/*
std::string SpeechCenterClient::loadGrammarContent(const std::string &grammarPath) {
    return readFileContent(grammarPath);
}
*/

speechcenter::recognizer::v1::RecognitionResource_Model SpeechCenterClient::convertTopicModel(const std::string &modelName) {
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

    return sanitize(content);
}

std::string SpeechCenterClient::sanitize(std::string str) {
    size_t endpos = str.find_last_not_of("\r\n");
    if(endpos != std::string::npos) {
        str.substr(0,endpos+1).swap(str);
    }
    return str;
}

void SpeechCenterClient::run(const Configuration &configuration) {
    connect(configuration);
    process(configuration);
}



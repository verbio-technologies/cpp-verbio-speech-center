#include "Configuration.h"

#include "gRpcExceptions.h"

#include <cxxopts.hpp>

#include <Audio.h>

#include <spdlog/spdlog.h>


Configuration::Configuration() : host("csr.api.speechcenter.verbio.com"), topic("generic"), language("en-US"),
                                 sampleRate(8000), chunkSize(20000), numberOfChannels(1) {}

Configuration::Configuration(int argc, char **argv) : Configuration() {
    parse(argc, argv);
}

Configuration::~Configuration() = default;

void Configuration::parse(int argc, char **argv) {

    cxxopts::Options options(argv[0], "Verbio Technlogies S.L. - Speech Center client example");
    options.set_width(180).allow_unrecognised_options().add_options()
            ("a,audio",
             "Path to a .wav audio in 8kHz or 16kHz sampling rate and PCM16 encoding to use for the recognition",
             cxxopts::value(audioPath), "file")
            //("g,grammar", "Path to the Grammar ABNF file to use for the recognition", cxxopts::value(grammarPath), "file")
            ("T,topic",
             "Topic to use for the recognition when a grammar is not provided. Must be GENERIC | BANKING | TELCO | INSURANCE",
             cxxopts::value(topic)->default_value(topic))
            ("l,language",
             "Language to use for the recognition: en-US, en-GB, pt-BR, es, es-419, tr, ja, fr, fr-CA, de, it",
             cxxopts::value(language)->default_value(language))
            ("t,token", "Path to the authentication token file", cxxopts::value(tokenPath))
            ("H,host", "URL of the Host or server trying to reach",
             cxxopts::value(host)->default_value("eu.speechcenter.verbio.com"))
            ("S,not-secure", "Toggle for non-secure GRPC connections",
             cxxopts::value<bool>(notSecure)->default_value("false"))
            ("d,diarization", "Toggle for diarization", cxxopts::value<bool>(diarization)->default_value("false"))
            ("f,formatting", "Toggle for formatting", cxxopts::value<bool>(formatting)->default_value("false"))
            ("A,asr-version", "Selectable asr version. Must be V1 | V2", cxxopts::value(asrVersion))
            ("c,chunk-size", "Size of audio chunk", cxxopts::value<uint32_t>(chunkSize)->default_value(std::to_string(chunkSize)))
            ("v,verbose", "Toogle for verbose output", cxxopts::value<bool>(verbose)->default_value("false"))
            ("h,help", "this help message");
    auto parsedOptions = options.parse(argc, argv);

    if (parsedOptions.count("h") > 0 || argc < 2) {
        std::cout << options.help();
        exit(0);
    }
    if ((parsedOptions.count("t") == 0) == (parsedOptions.count("g") == 0))
        throw GrpcException("Topic and grammar options are mutually exclusive and at least one is needed.");

    auto const &audio = Audio(audioPath);
    sampleRate = audio.getSamplingRate();
    numberOfChannels = audio.getChannels();

    if (verbose){
        spdlog::set_level(spdlog::level::trace);
    }
}

std::string Configuration::getAudioPath() const {
    return audioPath;
}

std::string Configuration::getHost() const {
    return host;
}

std::string Configuration::getLanguage() const {
    return language;
}

std::string Configuration::getTokenPath() const {
    return tokenPath;
}

std::string Configuration::getGrammarPath() const {
    return grammarPath;
}

std::string Configuration::getTopic() const {
    return topic;
}

bool Configuration::getNotSecure() const {
    return notSecure;
}

std::string Configuration::getAsrVersion() const {
    return asrVersion;
}

bool Configuration::getDiarization() const {
    return diarization;
}

bool Configuration::getFormatting() const {
    return formatting;
}

bool Configuration::getVerbosity() const {
    return verbose;
}

uint32_t Configuration::getChunkSize() const {
    return chunkSize;
}


uint32_t Configuration::getSampleRate() const {
    return sampleRate;
}

uint32_t Configuration::getNumberOfChannels() const {
    return numberOfChannels;
}
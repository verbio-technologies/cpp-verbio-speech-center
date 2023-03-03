#include "Configuration.h"

#include "gRpcExceptions.h"

#include <cxxopts.hpp>


Configuration::Configuration() : host("csr.api.speechcenter.verbio.com") , topic("generic"), language("en-US")
{}
Configuration::Configuration(int argc, char **argv) : Configuration() {
    parse(argc, argv);
}

Configuration::~Configuration() = default;

void Configuration::parse(int argc, char **argv) {

    cxxopts::Options options(argv[0], "Verbio Technlogies S.L. - Speech Center client example");
    options.set_width(180).allow_unrecognised_options().add_options()
            ("a,audio", "Path to a .wav audio in 8kHz and PCM16 encoding to use for the recognition", cxxopts::value(audioPath), "file")
            //("g,grammar", "Path to the Grammar ABNF file to use for the recognition", cxxopts::value(grammarPath), "file")
            ("T,topic", "Topic to use for the recognition when a grammar is not provided. Must be GENERIC | BANKING | TELCO | INSURANCE", cxxopts::value(topic)->default_value(topic))
            ("l,language", "Language to use for the recognition: en-US, en-GB, pt-BR, es, es-419, tr, ja, fr, fr-CA, de, it", cxxopts::value(language)->default_value(language))
            ("s,sample-rate", "Sample rate for the audio recognition: 8000, 16000.", cxxopts::value<uint32_t>(sampleRate))
            ("t,token", "Path to the authentication token file", cxxopts::value(tokenPath))
            ("H,host", "URL of the Host or server trying to reach", cxxopts::value(host)->default_value("eu.speechcenter.verbio.com"))
            ("S,not-secure", "Toggle for non-secure GRPC connections", cxxopts::value<bool>(notSecure)->default_value("false"))
            ("d,diarization", "Toggle for diarization", cxxopts::value<bool>(diarization)->default_value("false"))
            ("f,formatting", "Toggle for formatting", cxxopts::value<bool>(formatting)->default_value("false"))
            ("A,asr-version", "Selectable asr version. Must be V1 | V2", cxxopts::value(asrVersion))
            ("h,help", "this help message");
    auto parsedOptions = options.parse(argc, argv);

    if (parsedOptions.count("h") > 0) {
        std::cout << options.help();
        exit(0);
    }
    if ( (parsedOptions.count("t") == 0) == (parsedOptions.count("b") == 0))
        throw GrpcException("Topic and grammar options are mutually exclusive");
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

uint32_t Configuration::getSampleRate() const {
    return sampleRate;
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

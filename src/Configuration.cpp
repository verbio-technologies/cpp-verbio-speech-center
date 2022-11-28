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
            ("a", "Path to a .wav audio in 8kHz and PCM16 encoding to use for the recognition", cxxopts::value(audioPath), "file")
            ("g", "Path to the Grammar ABNF file to use for the recognition", cxxopts::value(grammarPath), "file")
            ("T", "Topic to use for the recognition when a grammar is not provided. Must be GENERIC | BANKING | TELCO | INSURANCE", cxxopts::value(topic)->default_value(topic))
            ("l", "Language to use for the recognition: es-ES, en-US or pt-BR.", cxxopts::value(language)->default_value(language))
            ("s", "Sample rate for the audio recognition: 16000.", cxxopts::value(sampleRate)->default_value(sampleRate))
            ("t", "Path to the authentication token file", cxxopts::value(tokenPath))
            ("H", "URL of the Host or server trying to reach", cxxopts::value(host)->default_value(host))
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

std::string Configuration::getSampleRate() const {
    return sampleRate;
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





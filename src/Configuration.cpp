#include "Configuration.h"

#include "gRpcExceptions.h"

#include <cxxopts.hpp>


Configuration::Configuration() : host("speechcenter.verbio.com:2424") , topic("generic"), language("en-US")
{}
Configuration::Configuration(int argc, char **argv) : Configuration() {
    parse(argc, argv);
}

Configuration::~Configuration() = default;

void Configuration::parse(int argc, char **argv) {

    cxxopts::Options options(argv[0], "Verbio Technlogies S.L. - Speech Center client example");
    options.set_width(180).allow_unrecognised_options().add_options()
            ("a", "Path to an audio file", cxxopts::value(audioPath), "file")
            ("b", "Path to a BNF grammar", cxxopts::value(grammarPath), "file")
            ("t", "Topic to use when a grammar is not provided: generic, telco or banking.", cxxopts::value(topic)->default_value(topic))
            ("l", "Language: es-ES, en-US or pt-BR.", cxxopts::value(language)->default_value(language))
            ("T", "Path to a token file", cxxopts::value(tokenPath))
            ("e", "End point to send requests", cxxopts::value(host)->default_value(host))
            ("h,help", "this help message");
    auto parsedOptions = options.parse(argc, argv);

    if (parsedOptions.count("h") > 0) {
        std::cout << options.help();
        exit(1);
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





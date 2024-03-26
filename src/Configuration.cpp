#include "Configuration.h"
#include "gRpcExceptions.h"
#include "logger.h"

#include <cxxopts.hpp>


Configuration::Configuration() : host("us.speechcenter.verbio.com"), language("en-US"),
                                 sampleRate(8000) {}

Configuration::Configuration(int argc, char **argv) : Configuration() {
    parse(argc, argv);
}

Configuration::~Configuration() = default;

void Configuration::parse(int argc, char **argv) {
    std::string grammarInline, grammarUri, grammarCompiled;

    cxxopts::Options options(argv[0], "Verbio Technlogies S.L. - Speech Center client example");
    options.set_width(180).allow_unrecognised_options().add_options()
            ("a,audio",
             "Path to a .wav audio in 8kHz or 16kHz sampling rate and PCM16 encoding to use for the recognition",
             cxxopts::value(audioPath), "file")
            ("I,inline-grammar", "ABNF Grammar to use for the recognition passed as a string", cxxopts::value(grammarInline), "string")
            ("G,grammar-uri", "Grammar URI to use for the recognition (builtin or externally served)", cxxopts::value(grammarUri), "uri")
            ("C,compiled-grammar", "Path to the compiled grammar file to use for the recognition", cxxopts::value(grammarCompiled), "file")
            ("T,topic",
             "Topic to use for the recognition when a grammar is not provided. Must be GENERIC | BANKING | TELCO | INSURANCE",
             cxxopts::value(topic))
            ("l,language",
             "Language to use for the recognition: en-US, en-GB, pt-BR, es, es-419, tr, ja, fr, fr-CA, de, it",
             cxxopts::value(language)->default_value(language))
            ("s,sample-rate", "Sampling rate for the audio recognition: 8000, 16000.",
             cxxopts::value<uint32_t>(sampleRate)->default_value(std::to_string(sampleRate)))
            ("t,token", "Path to the authentication token file", cxxopts::value(tokenPath))
            ("H,host", "URL of the Host or server trying to reach",
             cxxopts::value(host)->default_value("us.speechcenter.verbio.com"))
            ("S,not-secure", "Toggle for non-secure GRPC connections",
             cxxopts::value<bool>(notSecure)->default_value("false"))
            ("d,diarization", "Toggle for diarization", cxxopts::value<bool>(diarization)->default_value("false"))
            ("f,formatting", "Toggle for formatting", cxxopts::value<bool>(formatting)->default_value("false"))
            ("A,asr-version", "Selectable asr version. Must be V1 | V2", cxxopts::value(asrVersion))
            ("L,label", "Label for the request.", cxxopts::value(label)->default_value(""))
            ("client-id", "Client id for token refresh", cxxopts::value(clientId)->default_value(""))
            ("client-secret", "Client secret for token refresh", cxxopts::value(clientSecret)->default_value(""))
            ("h,help", "this help message");
    auto parsedOptions = options.parse(argc, argv);

    if (parsedOptions.count("h") > 0 || argc < 2) {
        std::cout << options.help();
        exit(0);
    }
    if (1 != (parsedOptions.count("T") + parsedOptions.count("G") + parsedOptions.count("I") + parsedOptions.count("C")))
        throw GrpcException("Topic and grammar options are mutually exclusive and at least one is needed.");

    if (parsedOptions.count("G") == 1) {
        grammar = Grammar(URI, grammarUri);
    }
    else if(parsedOptions.count("I") == 1) {
        grammar = Grammar(INLINE, grammarInline);
    }
    else if (parsedOptions.count("C") == 1) {
        grammar = Grammar(COMPILED, grammarCompiled);
    }
    else {
        grammar = Grammar();
    }

    validate_configuration_values();
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

bool Configuration::hasGrammar() const {
    return grammar.getType() != GrammarType::NONE;
}

Grammar Configuration::getGrammar() const {
    return grammar;
}

bool Configuration::hasTopic() const {
    return !topic.empty();
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

std::string Configuration::getLabel()  const {
    return label;
}

std::string Configuration::getClientId()  const {
    return clientId;
}

std::string Configuration::getClientSecret()  const {
    return clientSecret;
}

void Configuration::validate_configuration_values() {

    if(sampleRate != 8000 and sampleRate != 16000) {
        throw std::runtime_error("Unsupported parameter value. Allowed values sample rate: 8000 16000");
    }

    if (hasTopic())
        validate_string_value("topic", topic, allowedTopicValues);
    validate_string_value("language", language, allowedLanguageValues);
    validate_string_value("asr version", asrVersion, allowedAsrVersionValues);
}


void Configuration::validate_string_value(const char *name, const std::string &value, const std::vector<std::string> &allowedValues) {
    if (std::find(allowedValues.begin(), allowedValues.end(), value) == allowedValues.end())
    {
        std::stringstream ss;
        std::copy(allowedValues.begin(), allowedValues.end(), std::ostream_iterator<std::string>(ss, " "));
        std::string allowedValuesList = ss.str();
        std::string nameString(name);
        std::string errorMessage("Unsupported parameter value. Allowed values for ");
        std::string exceptionMessage = errorMessage + nameString.append(": ") + allowedValuesList;
        throw std::runtime_error(exceptionMessage);
    }
}
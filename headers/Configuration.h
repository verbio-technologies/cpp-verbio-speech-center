#ifndef VERBIO_ASRTESTCONFIGURATION_H
#define VERBIO_ASRTESTCONFIGURATION_H

#include <memory>
#include <string>
#include <vector>
#include "Grammar.h"

class Configuration {
public:
    Configuration();

    Configuration(int argc, char *argv[]);

    ~Configuration();

    void parse(int argc, char *argv[]);

    std::string getAudioPath() const;

    bool hasTopic() const;

    std::string getTopic() const;

    std::string getHost() const;

    std::string getLanguage() const;

    std::string getTokenPath() const;

    bool hasGrammar() const;

    Grammar getGrammar() const;

    uint32_t getSampleRate() const;

    bool getNotSecure() const;

    std::string getAsrVersion() const;

    bool getDiarization() const;

    bool getFormatting() const;

    std::string getLabel()  const;

    std::string getClientId()  const;

    std::string getClientSecret()  const;

    void validate_configuration_values();

private:

    void validate_string_value(const char *name, const std::string &value, const std::vector<std::string> &allowedValues);

    std::string language;
    std::string topic;
    Grammar grammar;
    std::string audioPath;
    std::string host;
    std::string tokenPath;
    std::string asrVersion;
    uint32_t sampleRate;
    bool notSecure;
    bool diarization;
    bool formatting;
    std::string label;
    std::string clientId;
    std::string clientSecret;
    std::vector<std::string> allowedTopicValues = {"GENERIC"};
    std::vector<std::string> allowedLanguageValues = {"en-US", "en-GB", "pt-BR", "es", "ca-ES", "es-419", "tr", "ja", "fr", "fr-CA", "de", "it"};
    std::vector<std::string> allowedAsrVersionValues = {"V1", "V2"};
    
};

#endif

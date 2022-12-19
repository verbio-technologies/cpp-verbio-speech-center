#ifndef VERBIO_ASRTESTCONFIGURATION_H
#define VERBIO_ASRTESTCONFIGURATION_H


#include <string>
#include <vector>
#include <memory>



class Configuration {
public:
    Configuration();
    Configuration(int argc, char* argv[]);
    ~Configuration();

    void parse(int argc, char* argv[]);

    std::string getAudioPath() const;
    std::string getTopic() const;
    std::string getHost() const;
    std::string getLanguage() const;
    std::string getTokenPath() const;
    std::string getGrammarPath() const;
    uint32_t getSampleRate() const;
    bool getNotSecure() const;

private:


    std::string language;
    std::string topic;
    std::string grammarPath;
    std::string audioPath;
    std::string host;
    std::string tokenPath;
    uint32_t sampleRate;
    bool notSecure;
};




#endif

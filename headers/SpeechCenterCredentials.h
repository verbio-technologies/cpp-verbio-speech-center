#ifndef CLI_CLIENT_SPEECHCENTERCREDENTIALS_H
#define CLI_CLIENT_SPEECHCENTERCREDENTIALS_H
#include <string>

class SpeechCenterCredentials {
public:
    SpeechCenterCredentials(std::string clientId,
                            std::string clientSecret,
                            std::string token,
                            std::string filePath);

    std::pair<bool, std::string> operator()();

    std::string newToken() const;

    int64_t expirationTime() const;

    void writeTokenFile(std::string token) const;

private:
    const std::string clientId;
    const std::string clientSecret;
    const std::string token;
    const std::string filePath;
    static const char* url;
};


#endif //CLI_CLIENT_SPEECHCENTERCREDENTIALS_H

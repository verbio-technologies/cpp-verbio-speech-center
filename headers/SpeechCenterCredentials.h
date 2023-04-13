#ifndef CLI_CLIENT_SPEECHCENTERCREDENTIALS_H
#define CLI_CLIENT_SPEECHCENTERCREDENTIALS_H
#include <string>


class SpeechCenterCredentials {
public:
    std::string newToken() const;

private:
    const std::string clientId;
    const std::string clientSecret;
    static constexpr char url[]{"https://auth.speechcenter.verbio.com:444/api/v1/token"};
};


#endif //CLI_CLIENT_SPEECHCENTERCREDENTIALS_H

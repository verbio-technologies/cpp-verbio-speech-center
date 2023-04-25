#ifndef CLI_CLIENT_SPEECHCENTERCREDENTIALS_H
#define CLI_CLIENT_SPEECHCENTERCREDENTIALS_H

#include <grpcpp/security/credentials.h>
#include <grpcpp/impl/codegen/client_context.h>
#include "recognition.pb.h"
#include "recognition.grpc.pb.h"
#include "Configuration.h"
#include <string>

class SpeechCenterCredentials {

public:

    explicit SpeechCenterCredentials(const std::string& tokenFilePath);
    void setClientCredentials(const std::string& client_id, const std::string& client_secret);

    std::string getToken();

private:

    std::string token;

    std::string clientId = {};

    std::string clientSecret = {};
    const std::string tokenFilePath;
    static const char* url;

    std::string requestNewToken() const;
    void writeTokenToFile(std::string token) const;
    int64_t decodeJwtExpirationTime() const;
    static std::string readFileContent(const std::string &path);


    static std::string sanitize(std::string str);
};


#endif //CLI_CLIENT_SPEECHCENTERCREDENTIALS_H

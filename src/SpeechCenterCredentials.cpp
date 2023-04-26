#include <thread>
#include <sstream>
#include <future>
#include <grpcpp/create_channel.h>
#include "Configuration.h"
#include "Audio.h"
#include "RecognitionClient.h"
#include "SpeechCenterCredentials.h"
#include <cpr/cpr.h>
#include "logger.h"
#include "jwt-cpp/jwt.h"
#include "jwt-cpp/traits/nlohmann-json/traits.h"
#include "gRpcExceptions.h"
#include <chrono>
#include "nlohmann/json.hpp"

const char* SpeechCenterCredentials::url = "https://auth.speechcenter.verbio.com:444/api/v1/token";

SpeechCenterCredentials::SpeechCenterCredentials(const std::string& tokenFilePath) : tokenFilePath{tokenFilePath} {

}

std::string SpeechCenterCredentials::getToken() {
    token = readFileContent(tokenFilePath);
    auto validUntil = decodeJwtExpirationTime();
    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (validUntil > now) return token;

    if(!clientId.empty() && !clientSecret.empty()){
        INFO("Current token expired or invalid, requesting a new token...");
        auto newToken = requestNewToken();
        writeTokenToFile(newToken);
        return newToken;
    }
    else {
        WARN("The token in file '{}' is expired but no client credentials have been provided. Cannot request a token refresh. Please log in to https://dashboard.speechcenter.verbio.com and retrieve your client credentials from the 'Credentials' page.", tokenFilePath);
    }
    return token;
}

std::string SpeechCenterCredentials::requestNewToken() const {

    std::string json{
            R"({"client_id":")"
            + clientId
            + R"(","client_secret":")"
            + clientSecret
            + "\"}"
    };

    auto response = cpr::Post(cpr::Url{url},
                              cpr::Header{{"Accept",       "application/json"},
                                          {"Content-Type", "application/json"}},
                              cpr::Body{json});

    DEBUG("Request for new token with response status code {}", response.status_code);

    auto credentials = nlohmann::json::parse(response.text);

    if (credentials.find("access_token") == credentials.end())
        throw GrpcException{"Access token not present in credentials returned by authorization server"};

    std::string new_token = credentials["access_token"];

    return new_token;
}

int64_t SpeechCenterCredentials::decodeJwtExpirationTime() const {
    try {
        const auto decoded = jwt::decode<jwt::traits::nlohmann_json>(token);
        auto claims = decoded.get_payload_claims();
        if (claims.find("exp") == claims.end())
            return 0;
        else
            return claims["exp"].as_int();

    }catch (...){
        return 0;
    }
}

void SpeechCenterCredentials::writeTokenToFile(std::string token) const {
    std::ofstream file{tokenFilePath};
    if(!file)
        throw IOError("Unable to open file '" + tokenFilePath + "'");
    file << token;
    file.close();
}

void SpeechCenterCredentials::setClientCredentials(const std::string &client_id, const std::string &client_secret) {
    clientId = client_id;
    clientSecret = client_secret;
}

std::string SpeechCenterCredentials::readFileContent(const std::string &path) {
    std::ifstream ifs(path);
    std::string content;
    if (ifs) {
        std::ostringstream oss;
        oss << ifs.rdbuf();
        content = oss.str();

    } else {
        throw IOError("Unable to open '" + path + "'");
    }

    return sanitize(content);
}

std::string SpeechCenterCredentials::sanitize(std::string str) {
    size_t endpos = str.find_last_not_of("\r\n");
    if (endpos != std::string::npos) {
        str.substr(0, endpos + 1).swap(str);
    }
    return str;
}
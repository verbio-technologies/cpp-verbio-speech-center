#include "SpeechCenterCredentials.h"
#include <cpr/cpr.h>
#include "logger.h"
#include "jwt-cpp/jwt.h"
#include "jwt-cpp/traits/nlohmann-json/traits.h"
#include "gRpcExceptions.h"
#include <chrono>
#include "nlohmann/json.hpp"

const char* SpeechCenterCredentials::url = "https://auth.speechcenter.verbio.com:444/api/v1/token";

SpeechCenterCredentials::SpeechCenterCredentials(const std::string clientId,
                                                 const std::string clientSecret,
                                                 const std::string token,
                                                 const std::string filePath)
        : clientId{clientId},
          clientSecret{clientSecret},
          token{token},
          filePath{filePath} {}

std::pair<bool, std::string> SpeechCenterCredentials::operator()() {

    auto validUntil = expirationTime();

    auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if( validUntil > now)
        return {false, ""};

    auto token = newToken();

    writeTokenFile(token);

    return {true, token};
}

std::string SpeechCenterCredentials::newToken() const {

    std::string json{
            "{\"client_id\":\""
            + clientId
            + "\",\"client_secret\":\""
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
        throw GrpcException{"access token not present in credentials returned by authorization server"};

    std::string new_token = credentials["access_token"];

    return new_token;
}

int64_t SpeechCenterCredentials::expirationTime() const {

    const auto decoded = jwt::decode<jwt::traits::nlohmann_json>(token);

    auto claims = decoded.get_payload_claims();

    if (claims.find("exp") == claims.end())
        throw GrpcException{"expiration claim not present in token"};

    return claims["exp"].as_int();
}

void SpeechCenterCredentials::writeTokenFile(std::string token) const {
    std::ofstream file{filePath};

    if(!file)
        throw IOError("Unable to open '" + filePath + "'");

    file << token;
}

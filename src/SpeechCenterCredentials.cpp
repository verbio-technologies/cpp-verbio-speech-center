#include "SpeechCenterCredentials.h"
#include <cpr/cpr.h>
#include "logger.h"

std::string SpeechCenterCredentials::newToken() const {

    auto response = cpr::Post(cpr::Url{url},
                              cpr::Header{{"Accept",       "application/json"},
                                          {"Content-Type", "application/json"}},
                              cpr::Payload{{"client_id",     clientId},
                                           {"client_secret", clientSecret}});


    DEBUG("Request for new token with response status code {}", response.status_code);

    return response.text;
}

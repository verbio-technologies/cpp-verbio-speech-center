#ifndef CLI_CLIENT_LOGGER_H
#define CLI_CLIENT_LOGGER_H


#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include <spdlog/spdlog.h>


#define INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define ERROR(...) SPDLOG_ERROR(__VA_ARGS__)


#endif

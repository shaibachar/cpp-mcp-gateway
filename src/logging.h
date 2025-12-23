#pragma once

#include <spdlog/spdlog.h>

#include <string>

// Centralized logger setup that reads configuration from environment variables.
// Supported variables:
// - GATEWAY_LOG_FILE: log file path (default: logs/gateway.log)
// - GATEWAY_LOG_LEVEL: debug|info|warn|error (default: info)
class SetupLogging {
  public:
    static void configure_from_env();
    static spdlog::level::level_enum parse_level(const std::string &level);
};

// Initialize spdlog with console and file sinks. The file will be appended.
// Call once at startup before emitting logs.
void init_logging(const std::string &log_file,
                  spdlog::level::level_enum level = spdlog::level::info);

inline void log_debug(const std::string &message) { spdlog::debug(message); }
inline void log_info(const std::string &message) { spdlog::info(message); }
inline void log_error(const std::string &message) { spdlog::error(message); }

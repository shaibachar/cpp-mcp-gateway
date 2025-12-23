#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

enum class LogLevel { Debug, Info, Error };

inline std::string log_level_label(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Error:
            return "ERROR";
    }
    return "UNKNOWN";
}

inline std::string current_timestamp() {
    using clock = std::chrono::system_clock;
    auto now = clock::now();
    auto time = clock::to_time_t(now);
    auto tm = *std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void log_message(LogLevel level, const std::string &message);

inline void log_debug(const std::string &message) { log_message(LogLevel::Debug, message); }
inline void log_info(const std::string &message) { log_message(LogLevel::Info, message); }
inline void log_error(const std::string &message) { log_message(LogLevel::Error, message); }

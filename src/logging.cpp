#include "logging.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <vector>

namespace {
std::string env_or_default(const char *name, const std::string &fallback) {
    const char *value = std::getenv(name);
    return (value && *value) ? std::string(value) : fallback;
}
}

spdlog::level::level_enum SetupLogging::parse_level(const std::string &level) {
    std::string lower = level;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (lower == "debug") return spdlog::level::debug;
    if (lower == "info") return spdlog::level::info;
    if (lower == "warn" || lower == "warning") return spdlog::level::warn;
    if (lower == "error") return spdlog::level::err;
    return spdlog::level::info;
}

void SetupLogging::configure_from_env() {
    auto log_file = env_or_default("GATEWAY_LOG_FILE", "logs/gateway.log");
    auto level_str = env_or_default("GATEWAY_LOG_LEVEL", "info");
    auto level = parse_level(level_str);

    std::filesystem::path log_path(log_file);
    if (log_path.has_parent_path() && !log_path.parent_path().empty()) {
        std::filesystem::create_directories(log_path.parent_path());
    }

    init_logging(log_file, level);
}

// Configure spdlog with both console and file sinks. The default logger is
// replaced so the inline helpers in logging.h forward correctly.
void init_logging(const std::string &log_file, spdlog::level::level_enum level) {
    std::vector<spdlog::sink_ptr> sinks;

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("%Y-%m-%d %H:%M:%S [%^%l%$] %v");
    sinks.push_back(console_sink);

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, true);
    file_sink->set_pattern("%Y-%m-%d %H:%M:%S [%l] %v");
    sinks.push_back(file_sink);

    auto logger = std::make_shared<spdlog::logger>("gateway", sinks.begin(), sinks.end());
    logger->set_level(level);
    spdlog::set_default_logger(logger);
    spdlog::set_level(level);
}

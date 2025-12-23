#include "logging.h"

#include <mutex>

namespace {
std::mutex &log_mutex() {
    static std::mutex mutex;
    return mutex;
}
}

void log_message(LogLevel level, const std::string &message) {
    std::lock_guard<std::mutex> lock(log_mutex());
    std::cout << current_timestamp() << " [" << log_level_label(level) << "] " << message << std::endl;
}

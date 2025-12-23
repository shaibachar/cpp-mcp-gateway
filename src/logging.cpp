#include "logging.h"

#include <mutex>

namespace {
// Internal accessor for a process-wide log mutex. No inputs; returns a static
// mutex reference. Only allocation failures during static initialization could
// throw.
std::mutex &log_mutex() {
    static std::mutex mutex;
    return mutex;
}
}

// Emit a log line with level and timestamp. Inputs: log level and message
// string. Output: none. Uses a mutex to serialize writes. Does not throw under
// normal conditions, though iostream operations may propagate exceptions if
// configured to do so.
void log_message(LogLevel level, const std::string &message) {
    // Serialize writes across threads to keep log lines intact.
    std::lock_guard<std::mutex> lock(log_mutex());
    std::cout << current_timestamp() << " [" << log_level_label(level) << "] " << message << std::endl;
}

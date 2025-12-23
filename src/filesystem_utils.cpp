#include "filesystem_utils.h"

#include "logging.h"

#include <fstream>

// Ensure a directory exists. Input: target path. Output: true when the path is
// a directory after the call. Exceptions are not thrown; error codes are used.
bool ensure_directory(const fs::path &path) {
    std::error_code ec;
    if (fs::exists(path, ec)) {
        return fs::is_directory(path, ec);
    }
    // Use create_directories to build intermediate paths when needed.
    return fs::create_directories(path, ec);
}

// Read a file into a string. Input: file path. Output: full file contents or an
// empty string if the file cannot be opened. Uses std::ifstream; exceptions are
// not explicitly thrown but allocation failures may propagate.
std::string read_file(const fs::path &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    // Stream into a string using iterators for simplicity and portability.
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

// Write content to disk. Inputs: destination path and content string. Output:
// true on success. Logs and returns false on failure. std::ofstream may throw
// on allocation or OS-level errors.
bool write_file(const fs::path &path, const std::string &content) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        log_error("Unable to open file for writing: " + path.string());
        return false;
    }

    file << content;
    return static_cast<bool>(file);
}

// Copy a file to a destination, creating parent directories. Inputs: source and
// destination paths. Output: true on success. Uses error codes to avoid
// throwing; may still propagate allocation exceptions.
bool copy_file_to(const fs::path &source, const fs::path &destination) {
    std::error_code ec;
    ensure_directory(destination.parent_path());
    fs::copy_file(source, destination, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        log_error("Failed to copy file from " + source.string() + " to " + destination.string() + ": " + ec.message());
        return false;
    }
    return true;
}

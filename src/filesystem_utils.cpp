#include "filesystem_utils.h"

#include "logging.h"

#include <fstream>

bool ensure_directory(const fs::path &path) {
    std::error_code ec;
    if (fs::exists(path, ec)) {
        return fs::is_directory(path, ec);
    }
    return fs::create_directories(path, ec);
}

std::string read_file(const fs::path &path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

bool write_file(const fs::path &path, const std::string &content) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        log_error("Unable to open file for writing: " + path.string());
        return false;
    }

    file << content;
    return static_cast<bool>(file);
}

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

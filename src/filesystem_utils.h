#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// Create the directory path if it does not already exist. Returns true when
// the path is a directory after the call completes.
bool ensure_directory(const fs::path &path);

// Read an entire file into a string with error code reporting. Returns true
// on success, false on failure. On failure, the error code is set appropriately.
bool read_file(const fs::path &path, std::string &content, std::error_code &ec);

// Write the provided content to disk at the given path. Returns true on
// success and logs errors on failure.
bool write_file(const fs::path &path, const std::string &content);

// Copy a file to the destination path, creating parent directories as needed.
// Overwrites existing files and logs errors on failure.
bool copy_file_to(const fs::path &source, const fs::path &destination);

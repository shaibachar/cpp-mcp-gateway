#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

bool ensure_directory(const fs::path &path);
std::string read_file(const fs::path &path);
bool write_file(const fs::path &path, const std::string &content);
bool copy_file_to(const fs::path &source, const fs::path &destination);

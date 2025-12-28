#include "filesystem_utils.h"

#include "logging.h"

#include <fstream>

// Ensure a directory exists. Input: target path. Output: true when the path is
// a directory after the call. Exceptions are not thrown; error codes are used.
bool ensure_directory(const fs::path &path)
{
    std::error_code ec;

    if (fs::exists(path, ec))
    {
        if (ec)
        {
            log_error("Error checking existence of path " + path.string() + ": " + ec.message());
            return false; // Check for errors
        }
        // Path exists, check if it's a directory
        return fs::is_directory(path, ec) && !ec;
    }

    if (ec)
    {
        log_error("Error " + path.string() + ": " + ec.message());
        return false; // exists() failed
    }
    // Path doesn't exist, create it
    fs::create_directories(path, ec);
    return !ec; // Return true only if no error occurred
}

// Read a file into a string with error code reporting. Input: file path. Output: true
// on success, false on failure. On failure, the error code is set appropriately.
bool read_file(const fs::path &path, std::string &content, std::error_code &ec)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
        return false;
    }

    content.assign((std::istreambuf_iterator<char>(file)),
                   std::istreambuf_iterator<char>());

    if (file.bad())
    {
        ec = std::make_error_code(std::errc::io_error);
        return false;
    }

    ec.clear();
    return true;
}

// Write content to disk. Inputs: destination path and content string. Output:
// true on success. Logs and returns false on failure. std::ofstream may throw
// on allocation or OS-level errors.
bool write_file(const fs::path &path, const std::string &content)
{
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        log_error("Unable to open file for writing: " + path.string());
        return false;
    }

    file << content;
    return static_cast<bool>(file);
}

// Copy a file to a destination, creating parent directories. Inputs: source and
// destination paths. Output: true on success. Uses error codes to avoid
// throwing; may still propagate allocation exceptions.
bool copy_file_to(const fs::path &source, const fs::path &destination)
{
    std::error_code ec;
    ensure_directory(destination.parent_path());
    fs::copy_file(source, destination, fs::copy_options::overwrite_existing, ec);
    if (ec)
    {
        log_error("Failed to copy file from " + source.string() + " to " + destination.string() + ": " + ec.message());
        return false;
    }
    return true;
}

bool is_writable_directory(const fs::path &path, std::string &message)
{
    if (!ensure_directory(path))
    {
        message = "Unable to create directory: " + path.string();
        return false;
    }

    std::error_code ec;
    if (!fs::is_directory(path, ec))
    {
        message = "Path is not a directory: " + path.string();
        return false;
    }

    auto temp_path = path / ".writetest.tmp";
    std::ofstream temp_file(temp_path, std::ios::out | std::ios::trunc);
    if (!temp_file.is_open())
    {
        message = "Directory is not writable: " + path.string();
        return false;
    }
    temp_file << "probe";
    temp_file.close();

    fs::remove(temp_path, ec);
    message.clear();
    return true;
}

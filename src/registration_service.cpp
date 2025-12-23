#include "registration_service.h"

#include "filesystem_utils.h"
#include "logging.h"

#include <fstream>
#include <sstream>

// Build a registration service configured with a mappings directory, an
// optional generation queue, and a validation strategy. Only allocation-related
// exceptions are expected from member initialization.
RegistrationService::RegistrationService(fs::path mappings_root, std::shared_ptr<GenerationQueue> generator, SpecValidator validator)
    : mappings_root_(std::move(mappings_root)), generator_(std::move(generator)), validator_(std::move(validator)) {}

// Validate and persist a specification. Inputs: version string and source file
// path. Output: RegistrationResult containing success flag, message, and stored
// path. Explicitly avoids throwing; errors are surfaced in the result object,
// though filesystem or allocation exceptions could still propagate.
RegistrationResult RegistrationService::register_spec(const std::string &version, const fs::path &source_path) {
    // Early validation ensures clear error messages before touching the
    // filesystem.
    if (version.empty()) {
        return {false, "Version is required", {}};
    }

    if (!fs::exists(source_path)) {
        return {false, "Spec file not found: " + source_path.string(), {}};
    }

    auto content = read_file(source_path);
    auto validation = validator_.validate(content);
    if (!validation.ok) {
        log_error("Validation failed for spec " + source_path.string() + ": " + validation.message);
        return {false, validation.message, {}};
    }

    fs::path target_dir = mappings_root_ / version;
    if (!ensure_directory(target_dir)) {
        return {false, "Unable to create mappings directory", {}};
    }

    fs::path destination = target_dir / source_path.filename();
    if (!copy_file_to(source_path, destination)) {
        return {false, "Failed to persist spec to mappings", {}};
    }

    log_info("Registered spec " + destination.string());

    if (generator_) {
        // Enqueue generation asynchronously so registration returns quickly.
        generator_->enqueue({version, destination});
    }

    return {true, "Registration accepted", destination};
}

#include "registration_service.h"

#include "filesystem_utils.h"
#include "logging.h"

#include <fstream>
#include <sstream>

RegistrationService::RegistrationService(fs::path mappings_root, std::shared_ptr<GenerationQueue> generator, SpecValidator validator)
    : mappings_root_(std::move(mappings_root)), generator_(std::move(generator)), validator_(std::move(validator)) {}

RegistrationResult RegistrationService::register_spec(const std::string &version, const fs::path &source_path) {
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
        generator_->enqueue({version, destination});
    }

    return {true, "Registration accepted", destination};
}

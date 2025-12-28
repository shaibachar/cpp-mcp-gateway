#pragma once

#include "generation_queue.h"
#include "metrics.h"
#include "spec_validation.h"

#include <filesystem>
#include <memory>
#include <string>

namespace fs = std::filesystem;

struct RegistrationResult {
    bool ok{false};
    std::string message;
    fs::path stored_path;
};

class RegistrationService {
  public:
    // Construct a registration service that persists specs beneath
    // mappings_root, validates them with the provided validator, and optionally
    // enqueues generation work on the supplied GenerationQueue.
    RegistrationService(fs::path mappings_root,
                        std::shared_ptr<GenerationQueue> generator,
                        std::shared_ptr<MetricsRegistry> metrics = nullptr,
                        SpecValidator validator = SpecValidator());

    // Persist and validate a spec for a version. On success, the spec is
    // copied into the mappings directory and a generation task is enqueued.
    RegistrationResult register_spec(const std::string &version, const fs::path &source_path);

  private:
    fs::path mappings_root_;
    std::shared_ptr<GenerationQueue> generator_;
    std::shared_ptr<MetricsRegistry> metrics_;
    SpecValidator validator_;
};

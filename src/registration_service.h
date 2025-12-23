#pragma once

#include "generation_queue.h"
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
    RegistrationService(fs::path mappings_root, std::shared_ptr<GenerationQueue> generator, SpecValidator validator = SpecValidator());

    RegistrationResult register_spec(const std::string &version, const fs::path &source_path);

  private:
    fs::path mappings_root_;
    std::shared_ptr<GenerationQueue> generator_;
    SpecValidator validator_;
};

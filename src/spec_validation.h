#pragma once

#include <cstddef>
#include <string>

struct ValidationResult {
    bool ok{false};
    std::string message;
};

class SpecValidator {
  public:
    // Create a validator that enforces a maximum payload size (in bytes) for
    // incoming specifications. Defaults to 10 MB to avoid runaway allocations.
    explicit SpecValidator(std::size_t max_bytes = 10 * 1024 * 1024);

    // Validate an OpenAPI specification payload. The validator ensures content
    // is non-empty, within the configured size limit, and appears to be an
    // OpenAPI 3.x document (rejecting Swagger 2.0).
    ValidationResult validate(const std::string &content) const;

  private:
    std::size_t max_bytes_;
};

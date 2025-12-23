#pragma once

#include <cstddef>
#include <string>

struct ValidationResult {
    bool ok{false};
    std::string message;
};

class SpecValidator {
  public:
    explicit SpecValidator(std::size_t max_bytes = 10 * 1024 * 1024);
    ValidationResult validate(const std::string &content) const;

  private:
    std::size_t max_bytes_;
};

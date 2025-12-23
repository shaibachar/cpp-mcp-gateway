#include "spec_validation.h"

#include <algorithm>

SpecValidator::SpecValidator(std::size_t max_bytes) : max_bytes_(max_bytes) {}

ValidationResult SpecValidator::validate(const std::string &content) const {
    if (content.empty()) {
        return {false, "Specification is empty"};
    }

    if (content.size() > max_bytes_) {
        return {false, "Specification exceeds maximum allowed size"};
    }

    auto lowered = content;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (lowered.find("openapi") == std::string::npos) {
        return {false, "Document does not appear to be an OpenAPI specification"};
    }

    if (lowered.find("swagger: 2") != std::string::npos) {
        return {false, "Swagger 2.0 documents are not supported"};
    }

    if (lowered.find("openapi: 3") == std::string::npos) {
        return {false, "Only OpenAPI 3.x documents are supported"};
    }

    return {true, "Valid specification"};
}

#pragma once

#include "logging.h"

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct OperationDescriptor {
    std::string version;
    std::string kit_name;
    std::string operation_id;
    fs::path manifest_path;
};

class RuntimeRegistry {
  public:
    explicit RuntimeRegistry(fs::path clientkit_root);

    void load();
    std::vector<OperationDescriptor> list_operations() const;
    std::optional<OperationDescriptor> find_operation(const std::string &operation_id) const;

  private:
    fs::path clientkit_root_;
    std::map<std::string, OperationDescriptor> operations_;
};

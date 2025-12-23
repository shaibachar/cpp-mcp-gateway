#pragma once

#include "logging.h"

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// OperationDescriptor captures metadata about a generated client kit entry and
// the operations it exposes. The manifest_path points to the generated
// manifest.txt written during client kit generation.
struct OperationDescriptor {
    std::string version;
    std::string kit_name;
    std::string operation_id;
    fs::path manifest_path;
};

class RuntimeRegistry {
  public:
    // Initialize a registry rooted at clientkit_root where generated client
    // kits are stored on disk. The registry can be reloaded multiple times to
    // pick up new kits without restarting the process.
    explicit RuntimeRegistry(fs::path clientkit_root);

    // Scan the client kit directory and rebuild the in-memory operation map.
    // The method is idempotent and safe to call before every operation lookup.
    void load();

    // Return a copy of the known operations in arbitrary iteration order. The
    // descriptors include version, kit name, operation id, and manifest path.
    std::vector<OperationDescriptor> list_operations() const;

    // Lookup a specific operation by identifier. Returns an optional descriptor
    // so callers can branch on existence without throwing.
    std::optional<OperationDescriptor> find_operation(const std::string &operation_id) const;

  private:
    fs::path clientkit_root_;
    std::map<std::string, OperationDescriptor> operations_;
};

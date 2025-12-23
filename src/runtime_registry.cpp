#include "runtime_registry.h"

#include <fstream>
#include <sstream>

// Create a registry rooted at a client kit directory. The constructor stores
// the path by value; only allocation failures could throw.
RuntimeRegistry::RuntimeRegistry(fs::path clientkit_root) : clientkit_root_(std::move(clientkit_root)) {}

// Refresh the registry from disk. No parameters; repopulates the internal map
// and logs informative messages. Returns void. Uses non-throwing filesystem
// operations where possible, but standard library exceptions may propagate if
// allocations fail.
void RuntimeRegistry::load() {
    // Clear previous state to guarantee the registry reflects the filesystem on
    // every invocation.
    operations_.clear();
    if (!fs::exists(clientkit_root_)) {
        log_info("No clientkit directory found at " + clientkit_root_.string());
        return;
    }

    for (const auto &version_entry : fs::directory_iterator(clientkit_root_)) {
        if (!version_entry.is_directory()) {
            continue;
        }
        auto version = version_entry.path().filename().string();
        for (const auto &kit_entry : fs::directory_iterator(version_entry)) {
            if (!kit_entry.is_directory()) {
                continue;
            }
            auto kit_name = kit_entry.path().filename().string();
            auto manifest_path = kit_entry.path() / "manifest.txt";
            if (!fs::exists(manifest_path)) {
                // Skip partially generated or invalid kits that do not include a
                // manifest file.
                continue;
            }

            std::ifstream manifest(manifest_path);
            std::string line;
            while (std::getline(manifest, line)) {
                const std::string prefix = "operation:";
                if (line.rfind(prefix, 0) == 0) {
                    auto op_id = line.substr(prefix.size());
                    OperationDescriptor descriptor{version, kit_name, op_id, manifest_path};
                    operations_[op_id] = descriptor;
                }
            }
        }
    }

    log_info("Loaded " + std::to_string(operations_.size()) + " operations from client kits");
}

// Retrieve a snapshot of loaded operations. Returns a vector copy of the
// descriptors and does not accept parameters. No explicit exceptions are
// thrown; allocation failures may propagate.
std::vector<OperationDescriptor> RuntimeRegistry::list_operations() const {
    // Return a copy to keep the internal cache encapsulated.
    std::vector<OperationDescriptor> list;
    list.reserve(operations_.size());
    for (const auto &entry : operations_) {
        list.push_back(entry.second);
    }
    return list;
}

// Find an operation by identifier. Accepts the operation id string and returns
// an optional descriptor populated when found. No explicit exceptions are
// thrown; standard library errors from map lookups or allocations may
// propagate.
std::optional<OperationDescriptor> RuntimeRegistry::find_operation(const std::string &operation_id) const {
    // Lookup without throwing to allow simple truthiness checks at call sites.
    auto it = operations_.find(operation_id);
    if (it != operations_.end()) {
        return it->second;
    }
    return std::nullopt;
}

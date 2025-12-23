#include "runtime_registry.h"

#include <fstream>
#include <sstream>

RuntimeRegistry::RuntimeRegistry(fs::path clientkit_root) : clientkit_root_(std::move(clientkit_root)) {}

void RuntimeRegistry::load() {
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

std::vector<OperationDescriptor> RuntimeRegistry::list_operations() const {
    std::vector<OperationDescriptor> list;
    list.reserve(operations_.size());
    for (const auto &entry : operations_) {
        list.push_back(entry.second);
    }
    return list;
}

std::optional<OperationDescriptor> RuntimeRegistry::find_operation(const std::string &operation_id) const {
    auto it = operations_.find(operation_id);
    if (it != operations_.end()) {
        return it->second;
    }
    return std::nullopt;
}

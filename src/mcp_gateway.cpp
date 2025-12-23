#include "mcp_gateway.h"

#include <sstream>

McpGateway::McpGateway(RuntimeRegistry registry) : registry_(std::move(registry)) {}

std::string McpGateway::list_operations() {
    registry_.load();
    std::ostringstream oss;
    for (const auto &operation : registry_.list_operations()) {
        oss << operation.operation_id << " (version: " << operation.version << ", kit: " << operation.kit_name << ")\n";
    }
    return oss.str();
}

std::string McpGateway::execute_operation(const std::string &operation_id, const std::string &payload) {
    registry_.load();
    auto op = registry_.find_operation(operation_id);
    if (!op) {
        return "Operation not found: " + operation_id;
    }

    std::ostringstream oss;
    oss << "Executed " << op->operation_id << " for version " << op->version << " with payload: " << payload;
    return oss.str();
}

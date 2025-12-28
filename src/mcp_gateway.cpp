#include "mcp_gateway.h"

#include <chrono>
#include <sstream>

// Construct a gateway that fronts the provided runtime registry. The registry
// is stored by value; no exceptions are thrown here beyond potential
// std::bad_alloc during copy.
McpGateway::McpGateway(RuntimeRegistry registry,
                       std::size_t max_concurrent_operations,
                       std::shared_ptr<MetricsRegistry> metrics)
    : registry_(std::move(registry)),
      max_concurrent_operations_(max_concurrent_operations),
      metrics_(std::move(metrics)) {}

// List known operations discovered by the registry. The method reloads the
// registry, takes no parameters, returns a newline-delimited string of
// operation summaries, and does not throw beyond standard library allocation
// failures.
std::string McpGateway::list_operations() {
    if (metrics_) {
        metrics_->record_mcp_list_request();
    }
    // Refresh the registry on each call so newly generated client kits are
    // discoverable without restarting the service.
    registry_.load();
    std::ostringstream oss;
    for (const auto &operation : registry_.list_operations()) {
        oss << operation.operation_id << " (version: " << operation.version << ", kit: " << operation.kit_name << ")\n";
    }
    return oss.str();
}

// Execute a simulated MCP operation lookup. Accepts an operation identifier and
// a payload string, returns a human-readable status describing the invocation,
// and returns a not-found message when the operation is missing. Exceptions are
// not explicitly thrown; standard library exceptions may propagate if string
// operations or registry interactions fail internally.
std::string McpGateway::execute_operation(const std::string &operation_id, const std::string &payload) {
    if (metrics_) {
        metrics_->record_mcp_execute_request();
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (active_ >= max_concurrent_operations_) {
            if (metrics_) {
                metrics_->record_mcp_execute_rejected();
            }
            return "Backpressure: too many concurrent operations";
        }
        ++active_;
    }

    auto start = std::chrono::steady_clock::now();
    auto finish = [this, start]() {
        if (metrics_) {
            auto end = std::chrono::steady_clock::now();
            auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            metrics_->record_mcp_execute_latency_ms(duration_ms);
        }
        std::lock_guard<std::mutex> lock(mutex_);
        if (active_ > 0) {
            --active_;
        }
    };

    // Ensure the registry is current before attempting an operation lookup.
    registry_.load();
    auto op = registry_.find_operation(operation_id);
    if (!op) {
        if (metrics_) {
            metrics_->record_mcp_execute_not_found();
        }
        finish();
        return "Operation not found: " + operation_id;
    }

    std::ostringstream oss;
    oss << "Executed " << op->operation_id << " for version " << op->version << " with payload: " << payload;
    if (metrics_) {
        metrics_->record_mcp_execute_success();
    }
    finish();
    return oss.str();
}

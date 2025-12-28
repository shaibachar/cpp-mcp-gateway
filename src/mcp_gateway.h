#pragma once

#include "metrics.h"
#include "runtime_registry.h"

#include <memory>
#include <mutex>
#include <string>

// McpGateway provides a thin façade over the runtime registry to expose
// Model Context Protocol style operations. It refreshes the registry on
// every call so that newly generated client kits are discoverable without
// restarting the process.
class McpGateway {
  public:
    // Construct a gateway with a concrete runtime registry. The registry is
    // stored by value so the gateway can manage lifecycle calls (e.g., load)
    // without assuming ownership semantics.
    explicit McpGateway(RuntimeRegistry registry,
                        std::size_t max_concurrent_operations = 8,
                        std::shared_ptr<MetricsRegistry> metrics = nullptr);

    // Return a formatted list of all operations known to the registry. Each
    // entry includes the operation identifier, the client kit version, and
    // the kit name extracted from the generated manifest.
    std::string list_operations();

    // Execute a simulated MCP operation lookup. If the operation exists the
    // method returns a descriptive string that echoes the payload; otherwise
    // it returns a human-friendly not-found message.
    std::string execute_operation(const std::string &operation_id, const std::string &payload);

  private:
    RuntimeRegistry registry_;
    std::size_t max_concurrent_operations_;
    std::shared_ptr<MetricsRegistry> metrics_;
    mutable std::mutex mutex_;
    std::size_t active_{0};
};

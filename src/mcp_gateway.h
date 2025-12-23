#pragma once

#include "runtime_registry.h"

#include <string>

class McpGateway {
  public:
    explicit McpGateway(RuntimeRegistry registry);

    std::string list_operations();
    std::string execute_operation(const std::string &operation_id, const std::string &payload);

  private:
    RuntimeRegistry registry_;
};

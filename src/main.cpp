#include "filesystem_utils.h"
#include "generation_queue.h"
#include "logging.h"
#include "mcp_gateway.h"
#include "metrics.h"
#include "registration_service.h"
#include "runtime_registry.h"

#include <filesystem>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

namespace fs = std::filesystem;

void print_usage() {
    std::cout << "cpp-mcp-gateway\n"
              << "Usage:\n"
              << "  cpp-mcp-gateway register <version> <spec_path>\n"
              << "  cpp-mcp-gateway list\n"
              << "  cpp-mcp-gateway execute <operation_id> <payload>\n"
              << "  cpp-mcp-gateway metrics\n"
              << "  cpp-mcp-gateway health\n";
}

std::optional<std::size_t> read_size_t_env(const char *name) {
    if (const char *value = std::getenv(name)) {
        try {
            return static_cast<std::size_t>(std::stoul(value));
        } catch (const std::exception &) {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    fs::path mappings_root{"mappings"};
    fs::path clientkit_root{"clientkit"};
    SetupLogging::configure_from_env();

    auto metrics = std::make_shared<MetricsRegistry>();
    auto max_queue_size = read_size_t_env("CPP_MCP_MAX_QUEUE_SIZE").value_or(32);
    auto max_concurrent_ops = read_size_t_env("CPP_MCP_MAX_CONCURRENT_OPS").value_or(8);

    auto generator = std::make_shared<GenerationQueue>(clientkit_root, 3, max_queue_size, metrics);
    generator->start();

    RegistrationService registration(mappings_root, generator, metrics);
    RuntimeRegistry registry(clientkit_root, metrics);
    McpGateway gateway(std::move(registry), max_concurrent_ops, metrics);

    std::string command = argv[1];

    if (command == "register") {
        if (argc < 4) {
            print_usage();
            return 1;
        }

        std::string version = argv[2];
        fs::path spec_path = argv[3];
        auto result = registration.register_spec(version, spec_path);
        generator->wait_for_idle();
        generator->stop();
        if (!result.ok) {
            std::cerr << "Registration failed: " << result.message << std::endl;
            return 1;
        }
        std::cout << "Registration succeeded for version " << version << " using " << result.stored_path << std::endl;
        return 0;
    }

    if (command == "list") {
        generator->stop();
        std::cout << gateway.list_operations();
        return 0;
    }

    if (command == "execute") {
        if (argc < 4) {
            print_usage();
            return 1;
        }
        std::string operation_id = argv[2];
        std::string payload = argv[3];
        generator->stop();
        std::cout << gateway.execute_operation(operation_id, payload) << std::endl;
        return 0;
    }

    if (command == "metrics") {
        generator->stop();
        auto stats = generator->stats();
        std::cout << metrics->to_prometheus();
        std::cout << "cpp_mcp_generation_queue_depth " << stats.queue_depth << "\n";
        std::cout << "cpp_mcp_generation_active " << stats.active << "\n";
        std::cout << "cpp_mcp_generation_queue_max " << stats.max_queue_size << "\n";
        return 0;
    }

    if (command == "health") {
        generator->stop();
        std::string mappings_message;
        std::string clientkit_message;
        bool mappings_ok = is_writable_directory(mappings_root, mappings_message);
        bool clientkit_ok = is_writable_directory(clientkit_root, clientkit_message);

        RuntimeRegistry health_registry(clientkit_root, metrics);
        health_registry.load();
        auto registry_stats = health_registry.stats();
        auto queue_stats = generator->stats();

        bool ok = mappings_ok && clientkit_ok;
        std::cout << "status: " << (ok ? "ok" : "degraded") << "\n";
        std::cout << "mappings.writable: " << (mappings_ok ? "true" : "false") << "\n";
        if (!mappings_ok) {
            std::cout << "mappings.message: " << mappings_message << "\n";
        }
        std::cout << "clientkit.writable: " << (clientkit_ok ? "true" : "false") << "\n";
        if (!clientkit_ok) {
            std::cout << "clientkit.message: " << clientkit_message << "\n";
        }
        std::cout << "generator.running: " << (queue_stats.running ? "true" : "false") << "\n";
        std::cout << "generator.queue_depth: " << queue_stats.queue_depth << "\n";
        std::cout << "generator.active: " << queue_stats.active << "\n";
        std::cout << "registry.operation_count: " << registry_stats.operation_count << "\n";
        std::cout << "registry.last_load_ms: " << registry_stats.last_load_latency_ms << "\n";
        return ok ? 0 : 1;
    }

    print_usage();
    generator->stop();
    return 1;
}

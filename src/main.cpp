#include "generation_queue.h"
#include "logging.h"
#include "mcp_gateway.h"
#include "registration_service.h"
#include "runtime_registry.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

namespace fs = std::filesystem;

void print_usage() {
    std::cout << "cpp-mcp-gateway\n"
              << "Usage:\n"
              << "  cpp-mcp-gateway register <version> <spec_path>\n"
              << "  cpp-mcp-gateway list\n"
              << "  cpp-mcp-gateway execute <operation_id> <payload>\n";
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    fs::path mappings_root{"mappings"};
    fs::path clientkit_root{"clientkit"};
    SetupLogging::configure_from_env();

    auto generator = std::make_shared<GenerationQueue>(clientkit_root);
    generator->start();

    RegistrationService registration(mappings_root, generator);
    RuntimeRegistry registry(clientkit_root);
    McpGateway gateway(registry);

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

    print_usage();
    generator->stop();
    return 1;
}

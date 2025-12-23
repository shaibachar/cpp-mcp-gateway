#include "generation_queue.h"
#include "logging.h"
#include "mcp_gateway.h"
#include "registration_service.h"
#include "runtime_registry.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>

namespace fs = std::filesystem;

fs::path make_unique_temp_dir(const std::string &name) {
    auto base = fs::temp_directory_path();
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    auto dir = base / (name + std::to_string(now));
    fs::create_directories(dir);
    return dir;
}

void write_spec(const fs::path &path) {
    std::ofstream out(path);
    out << "openapi: 3.0.0\n";
    out << "info:\n  title: Example\n  version: 1.0.0\n";
    out << "paths:\n";
    out << "  /hello:\n";
    out << "    get:\n";
    out << "      operationId: sayHello\n";
    out << "      responses:\n        '200':\n          description: ok\n";
}

int main() {
    auto temp_root = make_unique_temp_dir("cpp-mcp-gateway-");
    auto mappings_root = temp_root / "mappings";
    auto clientkit_root = temp_root / "clientkit";
    auto log_path = temp_root / "test.log";
    fs::create_directories(mappings_root);
    fs::create_directories(clientkit_root);
    init_logging(log_path.string(), spdlog::level::debug);

    auto generator = std::make_shared<GenerationQueue>(clientkit_root);
    generator->start();

    RegistrationService registration(mappings_root, generator);

    auto spec_path = temp_root / "example.yaml";
    write_spec(spec_path);

    auto result = registration.register_spec("v1", spec_path);
    assert(result.ok);
    assert(fs::exists(result.stored_path));

    generator->wait_for_idle();
    generator->stop();

    RuntimeRegistry registry(clientkit_root);
    registry.load();

    auto operations = registry.list_operations();
    assert(!operations.empty());
    assert(operations.front().operation_id == "sayHello");

    McpGateway gateway(registry);
    auto response = gateway.execute_operation("sayHello", "{}");
    assert(response.find("sayHello") != std::string::npos);

    spdlog::shutdown();  // Ensure log files are closed before cleanup on Windows.
    fs::remove_all(temp_root);
    return 0;
}

#include "filesystem_utils.h"
#include "generation_queue.h"
#include "logging.h"
#include "mcp_gateway.h"
#include "registration_service.h"
#include "runtime_registry.h"
#include "spec_validation.h"

#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <string>

namespace fs = std::filesystem;

namespace {

void set_env_var(const std::string &name, const std::string &value) {
#ifdef _WIN32
    _putenv_s(name.c_str(), value.c_str());
#else
    setenv(name.c_str(), value.c_str(), 1);
#endif
}

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
    out << "      responses:\n        '200':\n";
    out << "          description: ok\n";
}

std::string read_file_to_string(const fs::path &path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

struct LoggingGuard {
    ~LoggingGuard() { spdlog::shutdown(); }
};

} // namespace

TEST(SpecValidatorTest, RejectsInvalidContent) {
    SpecValidator validator(50);
    EXPECT_FALSE(validator.validate("").ok);
    EXPECT_FALSE(validator.validate(std::string(60, 'a')).ok);
    EXPECT_FALSE(validator.validate("not open api").ok);
    EXPECT_FALSE(validator.validate("swagger: 2").ok);
}

TEST(SpecValidatorTest, AcceptsValidOpenApi3) {
    SpecValidator validator;
    auto result = validator.validate("openapi: 3.0.1\ninfo: {}\n");
    EXPECT_TRUE(result.ok);
}

TEST(LoggingSetupTest, UsesEnvironmentConfiguration) {
    auto temp_root = make_unique_temp_dir("logging-");
    auto log_file = (temp_root / "env.log").string();
    set_env_var("GATEWAY_LOG_FILE", log_file);
    set_env_var("GATEWAY_LOG_LEVEL", "debug");
    SetupLogging::configure_from_env();
    LoggingGuard guard;

    log_info("hello");
    spdlog::default_logger()->flush();

    EXPECT_TRUE(fs::exists(log_file));
    auto contents = read_file_to_string(log_file);
    EXPECT_NE(contents.find("hello"), std::string::npos);

    spdlog::shutdown();
    fs::remove_all(temp_root);
}

TEST(FileSystemUtilsTest, ReadWriteAndCopy) {
    auto temp_root = make_unique_temp_dir("fs-");
    auto file_path = temp_root / "nested" / "file.txt";
    auto copy_path = temp_root / "copy" / "file.txt";

    EXPECT_TRUE(ensure_directory(file_path.parent_path()));
    EXPECT_TRUE(write_file(file_path, "data"));
    std::string content;
    std::error_code ec;
    EXPECT_TRUE(read_file(file_path, content, ec));
    EXPECT_EQ(content, "data");
    EXPECT_TRUE(copy_file_to(file_path, copy_path));
    EXPECT_TRUE(fs::exists(copy_path));

    fs::remove_all(temp_root);
}

TEST(RegistrationServiceTest, FailsWhenSpecMissing) {
    auto temp_root = make_unique_temp_dir("registration-");
    RegistrationService service(temp_root / "mappings", nullptr);
    auto result = service.register_spec("v1", temp_root / "absent.yaml");
    EXPECT_FALSE(result.ok);
    fs::remove_all(temp_root);
}

TEST(IntegrationTest, GeneratesClientKitAndExecutesOperation) {
    auto temp_root = make_unique_temp_dir("gateway-");
    auto mappings_root = temp_root / "mappings";
    auto clientkit_root = temp_root / "clientkit";
    fs::create_directories(mappings_root);
    fs::create_directories(clientkit_root);

    auto log_path = (temp_root / "test.log").string();
    set_env_var("GATEWAY_LOG_FILE", log_path);
    set_env_var("GATEWAY_LOG_LEVEL", "debug");
    SetupLogging::configure_from_env();
    LoggingGuard guard;

    auto generator = std::make_shared<GenerationQueue>(clientkit_root, 1);
    generator->start();

    RegistrationService registration(mappings_root, generator);

    auto spec_path = temp_root / "example.yaml";
    write_spec(spec_path);

    auto result = registration.register_spec("v1", spec_path);
    ASSERT_TRUE(result.ok);
    ASSERT_TRUE(fs::exists(result.stored_path));

    generator->wait_for_idle();
    generator->stop();

    RuntimeRegistry registry(clientkit_root);
    registry.load();

    auto op = registry.find_operation("sayHello");
    ASSERT_TRUE(op.has_value());
    EXPECT_EQ(op->version, "v1");
    EXPECT_EQ(op->kit_name, "example");

    McpGateway gateway(registry);
    auto listed = gateway.list_operations();
    EXPECT_NE(listed.find("sayHello"), std::string::npos);

    auto response = gateway.execute_operation("sayHello", "{}");
    EXPECT_NE(response.find("sayHello"), std::string::npos);

    auto missing = gateway.execute_operation("missing", "{}");
    EXPECT_NE(missing.find("Operation not found"), std::string::npos);

    spdlog::shutdown();
    fs::remove_all(temp_root);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

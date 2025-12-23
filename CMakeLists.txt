cmake_minimum_required(VERSION 3.16)
project(cpp-mcp-gateway VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(gateway_lib
    src/logging.cpp
    src/spec_validation.cpp
    src/filesystem_utils.cpp
    src/generation_queue.cpp
    src/registration_service.cpp
    src/runtime_registry.cpp
    src/mcp_gateway.cpp
)

target_include_directories(gateway_lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(cpp-mcp-gateway src/main.cpp)
target_link_libraries(cpp-mcp-gateway PRIVATE gateway_lib)

enable_testing()
add_executable(unit_tests tests/unit_tests.cpp)
target_link_libraries(unit_tests PRIVATE gateway_lib)
add_test(NAME unit_tests COMMAND unit_tests)

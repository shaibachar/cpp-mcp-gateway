# Runtime behavior

## Server startup

- On load, the gateway scans the `clientkit/` directory to discover generated C++ clients.
- Each client kit is mapped to an MCP-compatible interface, ready to serve requests.
- Info-level logs report server readiness and the list of registered Swagger sources.
- New client kits are loaded only on server startup; changes require a new instance or restart.

## MCP request handling

- The gateway exposes MCP endpoints that support:
  - `list_operations`: returns available operations from registered specs.
  - `execute_operation`: invokes a specific operation against the downstream service.
  - Other standard MCP behaviors as required by the MCP protocol surface.
- Routes are resolved from an in-memory cache populated at startup from the client kits.
- Requests are forwarded to the underlying service defined by the corresponding Swagger file.
- Downstream errors are proxied through to the MCP caller without normalization (status codes and bodies passed through as-is, except for transport-level wrapping if required by MCP).
- Debug-level logs capture routing decisions and interactions; info-level logs keep to high-level status.
- Authentication and authorization are not enforced by the gateway; upstream systems must handle auth for exposed APIs.

## Caching strategy

- Route lookups are cached in memory for fast resolution.
- Cache refresh occurs when the server reloads and rebuilds the registry from available client kits.

## Dockerized deployment

- The gateway is intended to run inside a container.
- The container image should include dependencies for:
  - The C++ server components.
  - Swagger/OpenAPI C++ code generation toolchain.
  - Runtime libraries required by generated client kits.
- Persistent volumes or bind mounts can be used for `mappings/` and `clientkit/` to retain state across container restarts.
- External filesystem storage should be mounted for `mappings/` (and `clientkit/` if persistence is desired) to allow sharing across instances.
- Required mounts should be writable and owned (or chmod/chown-adjusted) for the runtime user; document expected uid/gid if non-root.
- Base image/toolchain: use a supported Linux base (e.g., Debian/Ubuntu LTS) with OpenAPI Generator C++ REST SDK target prerequisites and a compiler/toolchain matching the generated client’s required C++ standard (e.g., C++17).

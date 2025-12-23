# Project overview

cpp-mcp-gateway is a gateway service that accepts OpenAPI (Swagger) definitions, generates C++ client kits from them, and exposes the described APIs through an MCP-facing interface. The gateway is designed to run as a Dockerized service so it can be deployed consistently across environments.

## Core responsibilities

- Provide a REST endpoint for registering Swagger/OpenAPI files.
- Store uploaded specifications under `mappings/<version>/` (for example, `mappings/v1/`, `mappings/v2/`), where the base path encodes the version.
- Compile uploaded specifications into C++ client kits using the Swagger/OpenAPI tooling, placing the results in `clientkit/`.
- Load the generated client kits during server startup and map them to the MCP client format at runtime.
- Expose an MCP endpoint that can execute registered API operations routed to the originating service.
- Cache route mappings in memory for fast lookup while servicing MCP requests.
- Emit logs that capture actions at debug level and summarize server state at info level.
- Delegate authentication and authorization to upstream systems; the gateway itself does not enforce authN/authZ.
- Run code generation asynchronously using the OpenAPI Generator C++ REST SDK target so registration calls are non-blocking.
- Maintain compatibility with OpenAPI 3.x inputs and generated clients built with a modern compiler/toolchain (e.g., C++17).

## High-level architecture

1. **Registration API**: Accepts OpenAPI files and persists them to the versioned `mappings/` directory tree.
2. **Code generation**: Invokes the Swagger/OpenAPI C++ generator to build client kits inside `clientkit/` from the stored specs.
3. **Startup loader**: On server load, scans generated client kits and binds them to MCP-friendly interfaces.
4. **MCP runtime**: Serves MCP requests by routing them to the appropriate generated client based on the in-memory registry and cache.
5. **Observability**: Uses logging to trace actions (debug) and provide concise lifecycle information (info), including server startup and the list of registered Swagger sources.

## Directory expectations

- `mappings/`: Holds uploaded OpenAPI files organized by version (e.g., `mappings/v1/`).
- `clientkit/`: Contains generated C++ client code produced from the stored Swagger/OpenAPI files.
- `spec/`: Houses project specifications and operational notes.

# cpp-mcp-gateway

cpp-mcp-gateway is a Docker-ready C++ microservice that accepts Swagger/OpenAPI 3.x definitions, generates C++ REST SDK client kits, and exposes those APIs through an MCP (Model Context Protocol) interface. It acts as a bridge between OpenAPI-driven services and MCP clients, handling registration, code generation, and runtime routing so downstream tools can call your REST endpoints via MCP.

## Why use this gateway?
- **OpenAPI → MCP bridge**: Upload Swagger/OpenAPI specs and get an MCP endpoint that proxies your APIs without manual wiring.
- **Automated C++ client generation**: Uses OpenAPI Generator (C++ REST SDK target) to produce typed client kits per spec version.
- **Versioned mappings**: Stores specs under `mappings/<version>/` so multiple API versions can coexist safely.
- **Startup discovery**: Loads generated client kits on boot and caches routes in memory for fast lookup.
- **Container-first**: Designed to run in Docker with volumes for persistent specs and generated clients.
- **Logging-first**: Emits debug logs for actions and info logs for lifecycle and registry summaries.

## Core architecture
1. **Registration API**: REST endpoint receives a Swagger/OpenAPI YAML upload (multi-part form) and persists it to `mappings/<version>/`. Duplicate filenames overwrite within the same version.
2. **Code generation**: An asynchronous worker invokes the OpenAPI Generator (C++ REST SDK) to build a client kit inside `clientkit/`, retrying failures with bounded attempts and cleaning partial output on failure.
3. **Startup loader**: On service startup, the gateway scans `clientkit/`, binds generated clients to MCP-compatible interfaces, and warms an in-memory route cache.
4. **MCP runtime**: Exposes MCP endpoints (e.g., `list_operations`, `execute_operation`) that route calls to the appropriate generated client and proxy downstream responses without normalization.

## Directory layout
- `mappings/`: Versioned OpenAPI uploads (e.g., `mappings/v1/`, `mappings/v2/`).
- `clientkit/`: Generated C++ REST SDK client code produced from stored specs.
- `spec/`: Project documentation and operational notes.

## Registration flow (REST)
- **Upload**: Send a multi-part form request with the OpenAPI 3.x YAML file and a `version` path segment.
- **Validation**: The gateway validates the document and enforces file size limits (recommended ≤ 10 MB). Swagger 2.0 is rejected with a clear HTTP 400/413 error body.
- **Persistence**: The file is stored under `mappings/<version>/`; newer uploads overwrite existing files in that version.
- **Generation**: An async job runs OpenAPI Generator (C++ REST SDK target) to create the client kit in `clientkit/`, logging status at debug level and outcomes at info level.
- **Availability**: Newly generated clients load on the next service start; restart to pick up changes.

## Runtime behavior (MCP)
- **Startup scanning**: On boot, discover generated client kits in `clientkit/` and register them with MCP.
- **Routing**: Requests are resolved via the in-memory cache built from discovered routes and forwarded to the downstream services defined in the original OpenAPI files.
- **Error handling**: Downstream HTTP status codes/bodies are proxied as-is (aside from transport wrapping required by MCP).
- **Auth**: Authentication and authorization are delegated to upstream systems; the gateway does not enforce authN/authZ.

## Running in Docker (example)
```bash
# Build the image (ensure OpenAPI Generator + C++ toolchain are available in the Dockerfile)
docker build -t cpp-mcp-gateway .

# Run with volumes for persistence
docker run \
  -p 8080:8080 \                       # registration/MCP ports (adjust as needed)
  -v $(pwd)/mappings:/app/mappings \   # persist uploaded specs
  -v $(pwd)/clientkit:/app/clientkit \ # persist generated clients
  cpp-mcp-gateway
```

## Development and testing checklist
- Unit test registry management, route mapping, and MCP translation utilities.
- Integration test the end-to-end path: register an OpenAPI spec, verify it is stored in `mappings/`, confirm client generation in `clientkit/`, restart, and invoke operations through MCP.
- Exercise retry/backoff logic for failed generations and ensure partial outputs are cleaned.
- Add logging assertions: debug for step-level actions, info for lifecycle summaries and registered Swagger list.
- (Optional) Container/Dockerfile linting, SBOM generation, and performance checks for startup scan time and routing latency.

## Notes for contributors
- Keep README and `spec/` docs aligned; update both when behavior changes (registration, generation, runtime, or Docker usage).
- Follow C++ REST SDK generator conventions when documenting or altering code generation steps.
- Prefer clear logging examples and configuration notes to help operators debug registration and routing flows.

## SEO-friendly keywords
OpenAPI gateway, Swagger gateway, MCP server, MCP client bridge, C++ REST SDK, OpenAPI Generator, API gateway for MCP, Swagger to MCP proxy, Dockerized OpenAPI service, versioned OpenAPI registry, generated C++ client kits, Model Context Protocol, REST-to-MCP adapter.

# Testing and coverage guidelines

## Scope

These guidelines describe the expected test coverage for the gateway, including its registration REST endpoint, code generation pipeline, runtime routing, and MCP exposure.

## Expectations

- **Unit tests**: Cover core utility functions, registry management, and mapping logic between generated clients and MCP interfaces.
- **Integration tests**: Validate end-to-end flows such as:
  - Registering an OpenAPI document and verifying it is persisted to `mappings/<version>/`.
  - Triggering code generation and confirming output in `clientkit/`.
  - Loading generated client kits on startup and exposing operations through the MCP endpoint.
  - Routing MCP calls to the appropriate generated client using the in-memory cache.
- **Contract tests**: Ensure generated client kits conform to the source OpenAPI definitions (e.g., request/response shapes).
- **Logging assertions**: Verify debug-level logs trace registration and routing actions, and info-level logs confirm startup readiness and registered Swagger list.
- **Performance checks**: Optional but recommended smoke tests for cache hit behavior and startup scanning of `clientkit/`.
- **Container/image checks**: Lint Dockerfile(s), validate base OS/toolchain selection, and generate an SBOM or similar provenance artifact.
- **Async generation queue tests**: Cover backpressure, retry limits, and failure cleanup scenarios for the code-generation job runner.

## Coverage goals

- Aim for meaningful coverage on business logic and routing decisions rather than exhaustive coverage on generated code.
- Prioritize scenarios that guard against regression in:
  - File handling and versioned storage conventions.
  - Route cache population and lookup.
  - MCP request translation to generated client calls.
- Startup-only load behavior when new client kits are added (reload requires new instance).
- Proxying downstream errors without normalization.
- Async generation lifecycle (enqueue, status, retry/backoff, failure cleanup).

## Out of scope

- Authentication and authorization flows are not enforced by this gateway and should be validated in upstream components.

## Performance targets (guidance)

- Define and track budgets for startup scan time over `clientkit/` (e.g., ≤ 5s for 50 specs) and end-to-end MCP routing latency under typical load (e.g., p95 ≤ 200ms excluding downstream service time).
- Include load-testing smoke cases that exercise cached vs. uncached routing paths and concurrent async generations (e.g., 5–10 parallel uploads).

## What I would add for production

For a production deployment, I would expand the testing guidance to include validation of metrics and health checks (e.g., scrape tests for Prometheus endpoints, synthetic health probes for generator readiness and clientkit integrity) and stress tests that exercise backpressure paths. This includes load tests that confirm registration rate limits and bounded queue behavior, plus runtime tests that verify circuit-breaker and concurrency limits are enforced under downstream failure conditions.

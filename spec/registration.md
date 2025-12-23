# Specification registration

## REST endpoint behavior

- **Purpose**: Accept Swagger/OpenAPI files (YAML) from clients to register APIs.
- **Upload format**: Accepts multi-part file uploads with YAML Swagger content.
- **Storage layout**: Files are persisted under `mappings/<version>/`, such as `mappings/v1/` or `mappings/v2/`, where the base path encodes the version.
- **Versioning**: The `<version>` segment is client-specified and should align with the API version encoded in the Swagger document; supported OpenAPI versions are 3.x (Swagger 2.0 uploads are rejected with a validation error).
- **Validation**: Incoming payloads should be validated as OpenAPI/Swagger documents before persistence and generation.
- **File constraints**: Maximum upload size should be enforced (e.g., 10 MB) and rejected with a clear HTTP 413/400 response and error body when exceeded or invalid.
- **Overwrite policy**: Duplicate filenames within the same version overwrite existing files; the new content takes effect only after restart/new instance load.

## Processing pipeline

1. Receive the OpenAPI file via the register endpoint.
2. Persist the file to the appropriate versioned folder inside `mappings/`.
3. Invoke the Swagger/OpenAPI C++ generator (C++ REST SDK target) asynchronously to produce a client kit, following https://openapi-generator.tech/docs/generators/cpp-restsdk.
4. Track the async job lifecycle: enqueue, run, emit status updates (log-based), and mark success/failure. Failed generations should be retried with bounded attempts and backoff (e.g., 3 tries, exponential backoff), cleaning partial `clientkit/` output on failure.
5. Emit debug logs for each action (received, stored, generation queued/completed/failed) and info logs summarizing successful registrations.

## Expected outcomes

- A new or updated OpenAPI file appears in `mappings/<version>/`.
- Generated C++ client code for the API appears in `clientkit/`.
- The registry of available APIs is refreshed on the next server load.
- Overwrites are allowed; newer uploads replace existing files but take effect only after a server restart or new instance startup.

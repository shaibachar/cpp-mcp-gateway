FROM ubuntu:22.04

# Install build dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        libcpprest-dev \
        ca-certificates && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Default logging configuration (can be overridden via environment).
ENV GATEWAY_LOG_FILE=/app/logs/gateway.log \
    GATEWAY_LOG_LEVEL=info

# Copy source
COPY . /app

# Configure and build
RUN cmake -S . -B build && \
    cmake --build build --config Release

# Provide a non-root user
RUN useradd -m gateway && chown -R gateway:gateway /app
USER gateway

# Default directories for mappings and generated client kits
VOLUME ["/app/mappings", "/app/clientkit"]

CMD ["/app/build/cpp-mcp-gateway"]

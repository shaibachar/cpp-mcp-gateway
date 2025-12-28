#pragma once

#include <atomic>
#include <sstream>
#include <string>

struct MetricsSnapshot {
    long long registrations_total{0};
    long long registrations_failed{0};
    long long registrations_validation_failed{0};
    long long generation_enqueued{0};
    long long generation_queue_full{0};
    long long generation_success{0};
    long long generation_failure{0};
    long long generation_latency_ms_total{0};
    long long generation_latency_samples{0};
    long long registry_loads{0};
    long long registry_load_latency_ms_total{0};
    long long registry_load_latency_samples{0};
    long long mcp_list_requests{0};
    long long mcp_execute_requests{0};
    long long mcp_execute_success{0};
    long long mcp_execute_not_found{0};
    long long mcp_execute_rejected{0};
    long long mcp_execute_latency_ms_total{0};
    long long mcp_execute_latency_samples{0};
};

class MetricsRegistry {
  public:
    void record_registration_attempt();
    void record_registration_failure();
    void record_registration_validation_failure();
    void record_generation_enqueued();
    void record_generation_queue_full();
    void record_generation_success();
    void record_generation_failure();
    void record_generation_latency_ms(long long duration_ms);
    void record_registry_load(long long duration_ms);
    void record_mcp_list_request();
    void record_mcp_execute_request();
    void record_mcp_execute_success();
    void record_mcp_execute_not_found();
    void record_mcp_execute_rejected();
    void record_mcp_execute_latency_ms(long long duration_ms);

    MetricsSnapshot snapshot() const;
    std::string to_prometheus() const;

  private:
    std::atomic<long long> registrations_total_{0};
    std::atomic<long long> registrations_failed_{0};
    std::atomic<long long> registrations_validation_failed_{0};
    std::atomic<long long> generation_enqueued_{0};
    std::atomic<long long> generation_queue_full_{0};
    std::atomic<long long> generation_success_{0};
    std::atomic<long long> generation_failure_{0};
    std::atomic<long long> generation_latency_ms_total_{0};
    std::atomic<long long> generation_latency_samples_{0};
    std::atomic<long long> registry_loads_{0};
    std::atomic<long long> registry_load_latency_ms_total_{0};
    std::atomic<long long> registry_load_latency_samples_{0};
    std::atomic<long long> mcp_list_requests_{0};
    std::atomic<long long> mcp_execute_requests_{0};
    std::atomic<long long> mcp_execute_success_{0};
    std::atomic<long long> mcp_execute_not_found_{0};
    std::atomic<long long> mcp_execute_rejected_{0};
    std::atomic<long long> mcp_execute_latency_ms_total_{0};
    std::atomic<long long> mcp_execute_latency_samples_{0};
};

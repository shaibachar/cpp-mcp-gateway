#include "metrics.h"

void MetricsRegistry::record_registration_attempt() { ++registrations_total_; }

void MetricsRegistry::record_registration_failure() { ++registrations_failed_; }

void MetricsRegistry::record_registration_validation_failure() { ++registrations_validation_failed_; }

void MetricsRegistry::record_generation_enqueued() { ++generation_enqueued_; }

void MetricsRegistry::record_generation_queue_full() { ++generation_queue_full_; }

void MetricsRegistry::record_generation_success() { ++generation_success_; }

void MetricsRegistry::record_generation_failure() { ++generation_failure_; }

void MetricsRegistry::record_generation_latency_ms(long long duration_ms) {
    generation_latency_ms_total_ += duration_ms;
    ++generation_latency_samples_;
}

void MetricsRegistry::record_registry_load(long long duration_ms) {
    ++registry_loads_;
    registry_load_latency_ms_total_ += duration_ms;
    ++registry_load_latency_samples_;
}

void MetricsRegistry::record_mcp_list_request() { ++mcp_list_requests_; }

void MetricsRegistry::record_mcp_execute_request() { ++mcp_execute_requests_; }

void MetricsRegistry::record_mcp_execute_success() { ++mcp_execute_success_; }

void MetricsRegistry::record_mcp_execute_not_found() { ++mcp_execute_not_found_; }

void MetricsRegistry::record_mcp_execute_rejected() { ++mcp_execute_rejected_; }

void MetricsRegistry::record_mcp_execute_latency_ms(long long duration_ms) {
    mcp_execute_latency_ms_total_ += duration_ms;
    ++mcp_execute_latency_samples_;
}

MetricsSnapshot MetricsRegistry::snapshot() const {
    MetricsSnapshot snapshot;
    snapshot.registrations_total = registrations_total_.load();
    snapshot.registrations_failed = registrations_failed_.load();
    snapshot.registrations_validation_failed = registrations_validation_failed_.load();
    snapshot.generation_enqueued = generation_enqueued_.load();
    snapshot.generation_queue_full = generation_queue_full_.load();
    snapshot.generation_success = generation_success_.load();
    snapshot.generation_failure = generation_failure_.load();
    snapshot.generation_latency_ms_total = generation_latency_ms_total_.load();
    snapshot.generation_latency_samples = generation_latency_samples_.load();
    snapshot.registry_loads = registry_loads_.load();
    snapshot.registry_load_latency_ms_total = registry_load_latency_ms_total_.load();
    snapshot.registry_load_latency_samples = registry_load_latency_samples_.load();
    snapshot.mcp_list_requests = mcp_list_requests_.load();
    snapshot.mcp_execute_requests = mcp_execute_requests_.load();
    snapshot.mcp_execute_success = mcp_execute_success_.load();
    snapshot.mcp_execute_not_found = mcp_execute_not_found_.load();
    snapshot.mcp_execute_rejected = mcp_execute_rejected_.load();
    snapshot.mcp_execute_latency_ms_total = mcp_execute_latency_ms_total_.load();
    snapshot.mcp_execute_latency_samples = mcp_execute_latency_samples_.load();
    return snapshot;
}

std::string MetricsRegistry::to_prometheus() const {
    auto snapshot = this->snapshot();
    std::ostringstream out;
    out << "cpp_mcp_registrations_total " << snapshot.registrations_total << "\n";
    out << "cpp_mcp_registrations_failed_total " << snapshot.registrations_failed << "\n";
    out << "cpp_mcp_registrations_validation_failed_total " << snapshot.registrations_validation_failed << "\n";
    out << "cpp_mcp_generation_enqueued_total " << snapshot.generation_enqueued << "\n";
    out << "cpp_mcp_generation_queue_full_total " << snapshot.generation_queue_full << "\n";
    out << "cpp_mcp_generation_success_total " << snapshot.generation_success << "\n";
    out << "cpp_mcp_generation_failure_total " << snapshot.generation_failure << "\n";
    out << "cpp_mcp_generation_latency_ms_total " << snapshot.generation_latency_ms_total << "\n";
    out << "cpp_mcp_generation_latency_ms_count " << snapshot.generation_latency_samples << "\n";
    out << "cpp_mcp_registry_loads_total " << snapshot.registry_loads << "\n";
    out << "cpp_mcp_registry_load_latency_ms_total " << snapshot.registry_load_latency_ms_total << "\n";
    out << "cpp_mcp_registry_load_latency_ms_count " << snapshot.registry_load_latency_samples << "\n";
    out << "cpp_mcp_mcp_list_requests_total " << snapshot.mcp_list_requests << "\n";
    out << "cpp_mcp_mcp_execute_requests_total " << snapshot.mcp_execute_requests << "\n";
    out << "cpp_mcp_mcp_execute_success_total " << snapshot.mcp_execute_success << "\n";
    out << "cpp_mcp_mcp_execute_not_found_total " << snapshot.mcp_execute_not_found << "\n";
    out << "cpp_mcp_mcp_execute_rejected_total " << snapshot.mcp_execute_rejected << "\n";
    out << "cpp_mcp_mcp_execute_latency_ms_total " << snapshot.mcp_execute_latency_ms_total << "\n";
    out << "cpp_mcp_mcp_execute_latency_ms_count " << snapshot.mcp_execute_latency_samples << "\n";
    return out.str();
}

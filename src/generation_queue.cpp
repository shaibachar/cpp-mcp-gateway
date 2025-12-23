#include "generation_queue.h"

#include "filesystem_utils.h"

#include <chrono>
#include <fstream>
#include <sstream>

// Construct a queue that targets a client kit root directory and caps retries
// per task. Parameters: output root path and maximum retries. Only allocation
// failures may throw during initialization.
GenerationQueue::GenerationQueue(fs::path clientkit_root, std::size_t max_retries)
    : clientkit_root_(std::move(clientkit_root)), max_retries_(max_retries) {}

// Destructor ensures the worker is stopped. No inputs; best-effort cleanup that
// should not throw.
GenerationQueue::~GenerationQueue() { stop(); }

// Start the worker thread. No parameters; returns void. No explicit exceptions,
// but std::thread construction could throw.
void GenerationQueue::start() {
    // Start the worker thread once; subsequent calls are no-ops.
    std::lock_guard<std::mutex> lock(mutex_);
    if (running_) {
        return;
    }
    stopping_ = false;
    running_ = true;
    worker_ = std::thread(&GenerationQueue::worker_loop, this);
}

void GenerationQueue::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // Signal the worker to drain outstanding tasks and exit.
        stopping_ = true;
    }
    cv_.notify_all();
    if (worker_.joinable()) {
        worker_.join();
    }
    running_ = false;
}

void GenerationQueue::enqueue(const GenerationTask &task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(task);
    }
    log_info("Queued generation for version " + task.version + " using spec " + task.spec_path.string());
    cv_.notify_one();
}

// Block until no tasks remain in the queue or are active. No parameters;
// returns void and should not throw under normal circumstances.
void GenerationQueue::wait_for_idle() {
    // Block until all queued tasks have been processed.
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() { return queue_.empty() && active_ == 0; });
}

// Background worker loop that processes tasks until stop is requested. Internal
// use only; exceptions from generation are not expected to propagate.
void GenerationQueue::worker_loop() {
    log_info("Generation worker started");
    while (true) {
        GenerationTask task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this]() { return stopping_ || !queue_.empty(); });
            if (stopping_ && queue_.empty()) {
                // Exit when no additional work remains.
                break;
            }
            task = queue_.front();
            queue_.pop();
            ++active_;
        }

        run_task_with_retries(task);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            --active_;
        }
        cv_.notify_all();
    }
    log_info("Generation worker stopped");
}

bool GenerationQueue::run_task_with_retries(const GenerationTask &task) {
    // Retry transient failures with a simple linear backoff.
    for (std::size_t attempt = 1; attempt <= max_retries_; ++attempt) {
        if (generate_client_kit(task)) {
            log_info("Successfully generated client kit for version " + task.version + " on attempt " + std::to_string(attempt));
            return true;
        }

        log_error("Generation attempt " + std::to_string(attempt) + " failed for " + task.spec_path.string());
        std::this_thread::sleep_for(std::chrono::milliseconds(50 * attempt));
    }

    log_error("Exhausted retries for " + task.spec_path.string());
    return false;
}

bool GenerationQueue::generate_client_kit(const GenerationTask &task) {
    // Validate spec existence.
    if (!fs::exists(task.spec_path)) {
        log_error("Spec file missing: " + task.spec_path.string());
        return false;
    }

    auto kit_name = task.spec_path.stem().string();
    fs::path output_dir = clientkit_root_ / task.version / kit_name;

    if (!ensure_directory(output_dir)) {
        log_error("Unable to create client kit directory: " + output_dir.string());
        return false;
    }

    auto operations = extract_operation_ids(task.spec_path);
    if (operations.empty()) {
        // Fall back to a default operation so the manifest is never empty.
        operations.push_back("default_operation");
    }

    fs::path manifest_path = output_dir / "manifest.txt";
    std::ostringstream manifest;
    manifest << "version:" << task.version << "\n";
    manifest << "spec:" << task.spec_path.string() << "\n";
    for (const auto &op : operations) {
        manifest << "operation:" << op << "\n";
    }

    if (!write_file(manifest_path, manifest.str())) {
        fs::remove_all(output_dir);
        return false;
    }

    fs::path cache_path = output_dir / "routes.cache";
    std::ostringstream cache;
    for (const auto &op : operations) {
        cache << op << " -> " << kit_name << "\n";
    }
    if (!write_file(cache_path, cache.str())) {
        fs::remove_all(output_dir);
        return false;
    }

    log_debug("Generated manifest at " + manifest_path.string());
    return true;
}

std::vector<std::string> GenerationQueue::extract_operation_ids(const fs::path &spec_path) const {
    std::ifstream file(spec_path);
    std::vector<std::string> operations;
    std::string line;
    while (std::getline(file, line)) {
        auto pos = line.find("operationId");
        if (pos == std::string::npos) {
            continue;
        }

        auto colon = line.find(":", pos);
        if (colon == std::string::npos) {
            continue;
        }

        auto value = line.substr(colon + 1);
        auto start = value.find_first_not_of(" \t\"'");
        auto end = value.find_last_not_of(" \t\"'");
        if (start != std::string::npos && end != std::string::npos && end >= start) {
            operations.emplace_back(value.substr(start, end - start + 1));
        }
    }
    return operations;
}

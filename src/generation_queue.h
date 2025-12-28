#pragma once

#include "logging.h"
#include "metrics.h"

#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

struct GenerationTask {
    std::string version;
    fs::path spec_path;
};

struct GenerationStatus {
    bool success{false};
    std::string message;
    fs::path output_dir;
};

class GenerationQueue {
  public:
    // Initialize a queue that writes client kits under clientkit_root. The
    // worker thread will retry failed generation attempts up to max_retries.
    GenerationQueue(fs::path clientkit_root,
                    std::size_t max_retries = 3,
                    std::size_t max_queue_size = 32,
                    std::shared_ptr<MetricsRegistry> metrics = nullptr);
    ~GenerationQueue();

    // Start the background worker thread. Safe to call multiple times; the
    // worker will only start once.
    void start();

    // Signal the worker thread to stop and join it. Additional enqueued tasks
    // will not be processed once stop has been requested.
    void stop();

    // Enqueue a new generation task with the target version and spec path. The
    // worker thread consumes tasks in FIFO order.
    bool enqueue(const GenerationTask &task);

    // Block the caller until the queue is empty and no tasks are active. Useful
    // for graceful shutdowns in tests or CLI flows.
    void wait_for_idle();

    struct Stats {
        std::size_t queue_depth{0};
        std::size_t active{0};
        std::size_t max_queue_size{0};
        bool running{false};
        bool stopping{false};
    };

    Stats stats() const;

  private:
    // Main worker loop that consumes tasks until stop is requested.
    void worker_loop();

    // Attempt to run a generation task with bounded retries. Returns true on
    // the first successful attempt.
    bool run_task_with_retries(const GenerationTask &task);

    // Perform the actual client kit generation. Returns false if the spec is
    // missing, the output directory cannot be created, or manifests fail to
    // persist.
    bool generate_client_kit(const GenerationTask &task);

    // Parse operation identifiers from the spec file. The parser is lightweight
    // and uses substring matching to find `operationId` entries.
    std::vector<std::string> extract_operation_ids(const fs::path &spec_path) const;

    fs::path clientkit_root_;
    std::size_t max_retries_;
    std::size_t max_queue_size_;
    std::queue<GenerationTask> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool running_{false};
    bool stopping_{false};
    std::thread worker_;
    std::size_t active_{0};
    std::shared_ptr<MetricsRegistry> metrics_;
};

#pragma once

#include "logging.h"

#include <condition_variable>
#include <filesystem>
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
    GenerationQueue(fs::path clientkit_root, std::size_t max_retries = 3);
    ~GenerationQueue();

    void start();
    void stop();
    void enqueue(const GenerationTask &task);
    void wait_for_idle();

  private:
    void worker_loop();
    bool run_task_with_retries(const GenerationTask &task);
    bool generate_client_kit(const GenerationTask &task);
    std::vector<std::string> extract_operation_ids(const fs::path &spec_path) const;

    fs::path clientkit_root_;
    std::size_t max_retries_;
    std::queue<GenerationTask> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool running_{false};
    bool stopping_{false};
    std::thread worker_;
    std::size_t active_{0};
};

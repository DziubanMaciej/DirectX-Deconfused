#pragma once

#include "Threading/BlockingQueue.h"

#include <atomic>
#include <thread>
#include <functional>

class BackgroundWorker {
public:
    using Task = std::function<void()>;
    struct TaskData {
        Task task;
        std::condition_variable *completeCV;
        std::atomic_bool *completed;
    };
    using TaskQueue = BlockingQueue<TaskData>;

    BackgroundWorker(TaskQueue &taskQueue, const std::atomic_bool &terminate);
    BackgroundWorker(BackgroundWorker &&other) {
        this->thread = std::move(other.thread);
    }
    BackgroundWorker &operator=(BackgroundWorker &&) = delete;
    ~BackgroundWorker();

private:
    static void work(TaskQueue &taskQueue, const std::atomic_bool &terminate);
    std::thread thread;
};

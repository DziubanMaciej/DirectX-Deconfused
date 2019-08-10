#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class BlockingQueue {
public:
    BlockingQueue() = default;
    BlockingQueue(const BlockingQueue &) = delete;
    BlockingQueue(BlockingQueue &&) = delete;

    void push(const T &value) {
        auto lock = this->lock();
        conditionVariable.notify_one();
        queue.push(value);
    }

    void push(T &&value) {
        auto lock = this->lock();
        conditionVariable.notify_one();
        queue.push(std::move(value));
    }

    bool blockingPop(T &result) {
        auto lock = this->lock();

        // Immediately take first element if available
        if (!empty()) {
            result = std::move(queue.front());
            queue.pop();
            return true;
        }

        // Wait for push notification
        conditionVariable.wait(lock);

        // Try taking first element once more
        if (!empty()) {
            result = std::move(queue.front());
            queue.pop();
            return true;
        }

        // Could not pop
        return false;
    }

    bool empty() {
        return queue.empty();
    }

    void notifyAll() {
        conditionVariable.notify_all();
    }

    void clear() {
        auto lock = this->lock();
        std::queue<T> empty;
        std::swap(queue, empty);
        conditionVariable.notify_all();
    }

private:
    std::queue<T> queue;
    std::mutex mutex;
    std::condition_variable conditionVariable;

    std::unique_lock<std::mutex> lock() {
        return std::unique_lock<std::mutex>{this->mutex};
    }
};

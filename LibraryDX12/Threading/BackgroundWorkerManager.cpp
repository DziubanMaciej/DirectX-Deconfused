#include "BackgroundWorkerManager.h"

BackgroundWorkerManager::BackgroundWorkerManager() {
    auto concurentThreadsSupported = std::thread::hardware_concurrency();
    if (concurentThreadsSupported == 0u) {
        concurentThreadsSupported = 1u;
    }

    for (auto i = 0u; i < concurentThreadsSupported; i++) {
        this->workers.emplace_back(taskQueue, terminate);
    }
}

BackgroundWorkerManager::~BackgroundWorkerManager() {
    terminate.store(true);
    taskQueue.clear();
    workers.clear();
}

void BackgroundWorkerManager::pushTask(BackgroundWorker::Task task) {
    BackgroundWorker::TaskData taskData{task, nullptr, nullptr};
    pushTask(taskData);
}

void BackgroundWorkerManager::pushTask(BackgroundWorker::Task task, std::atomic_bool &completed) {
    BackgroundWorker::TaskData taskData{task, nullptr, &completed};
    pushTask(taskData);
}

void BackgroundWorkerManager::pushTask(BackgroundWorker::Task task, std::condition_variable &completed) {
    BackgroundWorker::TaskData taskData{task, &completed, nullptr};
    pushTask(taskData);
}

void BackgroundWorkerManager::pushTask(BackgroundWorker::Task task, std::atomic_bool &completed, std::condition_variable &completedCV) {
    BackgroundWorker::TaskData taskData{task, &completedCV, &completed};
    pushTask(taskData);
}

void BackgroundWorkerManager::pushTask(BackgroundWorker::TaskData taskData) {
    taskQueue.push(taskData);
}

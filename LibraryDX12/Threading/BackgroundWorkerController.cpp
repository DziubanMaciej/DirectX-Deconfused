#include "BackgroundWorkerController.h"

BackgroundWorkerController::BackgroundWorkerController() {
    auto concurentThreadsSupported = std::thread::hardware_concurrency();
    if (concurentThreadsSupported == 0u) {
        concurentThreadsSupported = 1u;
    }

    for (auto i = 0u; i < concurentThreadsSupported; i++) {
        this->workers.emplace_back(taskQueue, terminate);
    }
}

BackgroundWorkerController::~BackgroundWorkerController() {
    terminate.store(true);
    taskQueue.clear();
    workers.clear();
}

void BackgroundWorkerController::pushTask(BackgroundWorker::Task task) {
    BackgroundWorker::TaskData taskData{task, nullptr, nullptr};
    pushTask(taskData);
}

void BackgroundWorkerController::pushTask(BackgroundWorker::Task task, std::atomic_bool &completed) {
    BackgroundWorker::TaskData taskData{task, nullptr, &completed};
    pushTask(taskData);
}

void BackgroundWorkerController::pushTask(BackgroundWorker::Task task, std::condition_variable &completed) {
    BackgroundWorker::TaskData taskData{task, &completed, nullptr};
    pushTask(taskData);
}

void BackgroundWorkerController::pushTask(BackgroundWorker::Task task, std::atomic_bool &completed, std::condition_variable &completedCV) {
    BackgroundWorker::TaskData taskData{task, &completedCV, &completed};
    pushTask(taskData);
}

void BackgroundWorkerController::pushTask(BackgroundWorker::TaskData taskData) {
    taskQueue.push(taskData);
}

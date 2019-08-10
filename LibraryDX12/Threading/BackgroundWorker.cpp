#include "BackgroundWorker.h"

BackgroundWorker::BackgroundWorker(TaskQueue &taskQueue, const std::atomic_bool &terminate)
    : thread(work, std::reference_wrapper<TaskQueue>(taskQueue), std::reference_wrapper<const std::atomic_bool>(terminate)) {
}

BackgroundWorker::~BackgroundWorker() {
    if (thread.joinable()) {
        thread.join();
    }
}

void BackgroundWorker::work(TaskQueue &taskQueue, const std::atomic_bool &terminate) {
    while (!terminate.load()) {
        // Try to pop task from queue
        TaskData taskData = {};
        const bool obtained = taskQueue.blockingPop(taskData);

        if (obtained) {
            // Execute task
            taskData.task();

            // Signal completion to user
            if (taskData.completed) {
                taskData.completed->store(true);
            }
            if (taskData.completeCV) {
                taskData.completeCV->notify_one();
            }
        }
    }
}

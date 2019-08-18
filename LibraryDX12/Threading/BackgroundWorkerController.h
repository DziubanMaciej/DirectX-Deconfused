#pragma once

#include "BackgroundWorker.h"

#include "Threading/BlockingQueue.h"

#include <atomic>
#include <vector>

/// \brief Manages multiple background thread workers performing tasks
///
/// Controller creates N threads where N is number of hardware threads supported by current platform.
/// Each thread is wrapped by BackgroundWorker class which extracts tasks from centralized BlockingQueue
/// contained in the manager.
///
/// When BackgroundWorkerController is destroyed, it clears the queue (discarding all undone tasks), sets
/// terminate flag to let the workers now they should end execution and notify all workers, so they
/// are awaken if waiting blocked for new tasks.
///
/// User can select how they want to be notified about completion - setting atomic_bool to true,
/// notifying condition_variable, none or both
class BackgroundWorkerController {
public:
    BackgroundWorkerController();
    ~BackgroundWorkerController();

    void pushTask(BackgroundWorker::Task task);
    void pushTask(BackgroundWorker::Task task, std::atomic_bool &completed);
    void pushTask(BackgroundWorker::Task task, std::condition_variable &completed);
    void pushTask(BackgroundWorker::Task task, std::atomic_bool &completed, std::condition_variable &completedCV);
    void pushTask(BackgroundWorker::TaskData taskData);

private:
    std::vector<BackgroundWorker> workers = {};
    BackgroundWorker::TaskQueue taskQueue = {};
    std::atomic_bool terminate = false;
};

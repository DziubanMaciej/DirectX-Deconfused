#pragma once

#include "BackgroundWorker.h"

#include "Threading/BlockingQueue.h"

#include <atomic>
#include <vector>

class Event;

/// \brief Manages multiple background thread workers performing tasks
///
/// Controller creates N threads where N is number of hardware threads supported by current platform.
/// Each thread is wrapped by BackgroundWorker class which extracts tasks from centralized BlockingQueue
/// contained in the manager.
///
/// When BackgroundWorkerController is destroyed, it clears the queue (discarding all undone tasks), sets
/// terminate flag to let the workers know they should end execution and notify all workers, so they
/// are awaken if waiting blocked for new tasks.
///
/// User can select how they want to be notified about completion - setting atomic_bool to true,
/// notifying an Event, none or both
class BackgroundWorkerController {
public:
    BackgroundWorkerController();
    ~BackgroundWorkerController();

    void pushTask(BackgroundWorker::Task task);
    void pushTask(BackgroundWorker::Task task, std::atomic_bool &completed);
    void pushTask(BackgroundWorker::Task task, Event &completedEvent);
    void pushTask(BackgroundWorker::Task task, std::atomic_bool &completed, Event &completedEvent);
    void pushTask(BackgroundWorker::TaskData taskData);

private:
    std::vector<BackgroundWorker> workers = {};
    BackgroundWorker::TaskQueue taskQueue = {};
    std::atomic_bool terminate = false;
};

#pragma once

#include "Application/ApplicationImpl.h"
#include "Utility/ThrowIfFailed.h"

#include <atomic>
#include <cassert>
#include <mutex>

template <typename CpuLoadArgs, typename CpuLoadResult>
class CpuGpuOperation {
public:
    enum class AsyncLoadingStatus {
        NOT_STARTED,
        CPU_LOAD,
        CPU_LOAD_TERMINATED,
        CPU_LOAD_FAIL,
        GPU_LOAD_STARTING,
        GPU_LOAD,
        SUCCESS,
    };

    /// Main entrypoint to start the synchronous operation.
    /// \param implementation-defined arguments for the operation
    void runSynchronously(const CpuLoadArgs &args) {
        runImpl(args);
    }

    /// Main entrypoint to start the synchronous operation.
    /// \param implementation-defined arguments for the operation
    void runAsynchronously(const CpuLoadArgs &args) {
        auto task = [this, args]() { runImpl(args); };
        ApplicationImpl::getInstance().getBackgroundWorkerController().pushTask(task);
    }

    /// Used by to check whether both CPU and GPU phase has ended successfuly.
    /// \return true if results of processing are available
    bool isReady() {
        switch (this->status) {
        case AsyncLoadingStatus::NOT_STARTED:
        case AsyncLoadingStatus::CPU_LOAD:
        case AsyncLoadingStatus::CPU_LOAD_TERMINATED:
        case AsyncLoadingStatus::CPU_LOAD_FAIL:
        case AsyncLoadingStatus::GPU_LOAD_STARTING:
            return false;
        case AsyncLoadingStatus::GPU_LOAD:
            if (this->hasGpuLoadEnded()) {
                this->status = AsyncLoadingStatus::SUCCESS;
                return true;
            }
            return false;
        case AsyncLoadingStatus::SUCCESS:
            return true;
        default:
            UNREACHABLE_CODE();
        }
    }

    /// Sets flags for CPU load termination, which should be occasionally checked by implementations
    /// if they have been terminated during their CPU phase with isCpuLoadTerminated call and return
    /// early. Results of terminated CPU load are ignored and GPU phase is not initiated. GPU load
    /// cannot be terminated during execution and has to be waited for.
    /// \param blocking flag makes the method wait for one of the final statuses indicating end of processing
    void terminate(bool blocking) {
        {
            std::lock_guard<std::mutex> lock{this->terminateLock};
            shouldTerminate.store(true);
        }

        if (blocking) {
            while (true) {
                isReady();
                switch (status.load()) {
                case AsyncLoadingStatus::CPU_LOAD_TERMINATED:
                case AsyncLoadingStatus::CPU_LOAD_FAIL:
                case AsyncLoadingStatus::SUCCESS:
                    return;
                }
            }
        }
    }

protected:
    /// CPU phase to be implemented by subclasses. Implementations should check termination status
    /// (see terminate).
    /// \param args implementation-defined arguments
    /// \return implementation-defined results
    virtual CpuLoadResult cpuLoad(const CpuLoadArgs &args) = 0;

    /// Check if CPU phase succeeded. It's not called, if the loading has been terminated
    /// \param cpuLoadResult results returned by cpuLoad
    /// \return true if loading succeeded
    virtual bool isCpuLoadSuccessful(const CpuLoadResult &cpuLoadResult) = 0;

    /// GPU phase to be implemented by subclasses. Is not called
    /// \param cpuLoadResult results returned by cpuLoad
    virtual void gpuLoad(const CpuLoadResult &args) = 0;

    /// Implementation-defined check to see if asynchronous GPU processing is done.
    virtual bool hasGpuLoadEnded() = 0;

    /// Used by the subclasses during CPU load to check if the loading has been externally terminated
    /// by the terminate call
    /// \return true if the operation should be terminated early
    bool isCpuLoadTerminated() const {
        return shouldTerminate.load();
    }

private:
    void runImpl(const CpuLoadArgs &cpuLoadArgs) {
        // Enter CPU phase or return early
        {
            std::lock_guard<std::mutex> lock{this->terminateLock};
            if (isCpuLoadTerminated()) {
                status = AsyncLoadingStatus::CPU_LOAD_TERMINATED;
                return;
            }
            status = AsyncLoadingStatus::CPU_LOAD;
        }

        // Run CPU load phase
        CpuLoadResult cpuLoadResult = cpuLoad(cpuLoadArgs);

        // Enter GPU phase or return early
        {
            std::lock_guard<std::mutex> lock{this->terminateLock};
            if (isCpuLoadTerminated()) {
                status = AsyncLoadingStatus::CPU_LOAD_TERMINATED;
                return;
            }
            if (!isCpuLoadSuccessful(cpuLoadResult)) {
                this->status = AsyncLoadingStatus::CPU_LOAD_FAIL;
                return;
            }
            status = AsyncLoadingStatus::GPU_LOAD_STARTING;
        }

        // Run GPU load phase
        gpuLoad(cpuLoadResult);
        status = AsyncLoadingStatus::GPU_LOAD;
    }

    std::atomic<AsyncLoadingStatus> status = AsyncLoadingStatus::NOT_STARTED;
    std::atomic_bool shouldTerminate = false;
    std::mutex terminateLock{};
};

#pragma once

#include "Utility/ThrowIfFailed.h"

#include <atomic>
#include <mutex>

template <typename CpuLoadArgs, typename CpuLoadResult, typename GpuLoadArgs>
class AsyncLoadableObject {
protected:
    enum class AsyncLoadingStatus {
        NOT_STARTED,
        CPU_LOAD,
        GPU_LOAD,
        SUCCESS,
        FAIL,
    };

    void cpuGpuLoad(const CpuLoadArgs &args, bool asynchronousLoading) {
        if (asynchronousLoading) {
            auto task = [this, args]() {
                cpuGpuLoadImpl(args);
            };
            ApplicationImpl::getInstance().getBackgroundWorkerController().pushTask(task);
        } else {
            cpuGpuLoadImpl(args);
        }
    }

    virtual CpuLoadResult cpuLoad(const CpuLoadArgs &args) = 0;
    virtual bool isCpuLoadSuccessful(const CpuLoadResult &result) { return true; }
    virtual GpuLoadArgs createArgsForGpuLoad(const CpuLoadResult &cpuLoadResult) = 0;
    virtual void gpuLoad(const GpuLoadArgs &args) = 0;
    virtual bool hasGpuLoadEnded() = 0;

    void terminateBackgroundProcessing(bool blocking) {
        terminate.store(true);
        if (blocking) {
            AsyncLoadingStatus currentStatus;
            do {
                currentStatus = status.load();
                isReady();
            } while (currentStatus != AsyncLoadingStatus::SUCCESS && currentStatus != AsyncLoadingStatus::FAIL);
        }
    }

    bool shouldBackgroundProcessingTerminate() const {
        return terminate.load();
    }

    std::atomic_bool terminate = false;
    std::atomic<AsyncLoadingStatus> status = AsyncLoadingStatus::NOT_STARTED;

public:
    bool isReady() {
        switch (this->status) {
        case AsyncLoadingStatus::NOT_STARTED:
        case AsyncLoadingStatus::CPU_LOAD:
        case AsyncLoadingStatus::FAIL:
            return false;
        case AsyncLoadingStatus::GPU_LOAD: {
            std::lock_guard<std::mutex> lock{this->gpuLoadLock};
            if (this->hasGpuLoadEnded()) {
                this->status = AsyncLoadingStatus::SUCCESS;
                return true;
            }
            return false;
        }
        case AsyncLoadingStatus::SUCCESS:
            return true;
        default:
            UNREACHABLE_CODE();
        }
    }

private:
    void cpuGpuLoadImpl(const CpuLoadArgs &cpuLoadArgs) {
        status = AsyncLoadingStatus::CPU_LOAD;
        CpuLoadResult cpuLoadResult = cpuLoad(cpuLoadArgs);
        if (!isCpuLoadSuccessful(cpuLoadResult)) {
            this->status = AsyncLoadingStatus::FAIL;
            return;
        }

        GpuLoadArgs gpuLoadArgs = createArgsForGpuLoad(cpuLoadResult);
        std::unique_lock<std::mutex> gpuLoadLock{this->gpuLoadLock};
        gpuLoad(gpuLoadArgs);
        gpuLoadLock.unlock();
        status = AsyncLoadingStatus::GPU_LOAD;
    }

    std::mutex gpuLoadLock;
};

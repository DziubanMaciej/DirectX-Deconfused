#pragma once

#include "Utility/ThrowIfFailed.h"

#include <atomic>

template <typename CpuLoadArgs, typename CpuLoadResult, typename GpuLoadArgs, typename GpuLoadResult>
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
    virtual GpuLoadResult gpuLoad(const GpuLoadArgs &args) = 0;
    virtual bool isGpuLoadSuccessful(const GpuLoadResult &result) { return true; }
    virtual bool hasGpuLoadEnded() = 0;
    virtual void writeCpuGpuLoadResults(CpuLoadResult &cpuLoadResult, GpuLoadResult &gpuLoadResult) = 0;

    void terminateBackgroundProcessing(bool blocking) {
        terminate.store(true);
        if (blocking) {
            AsyncLoadingStatus currentStatus;
            do {
                currentStatus = status.load();
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

private:
    void cpuGpuLoadImpl(const CpuLoadArgs &cpuLoadArgs) {
        status = AsyncLoadingStatus::CPU_LOAD;
        CpuLoadResult cpuLoadResult = cpuLoad(cpuLoadArgs);
        if (!isCpuLoadSuccessful(cpuLoadResult)) {
            this->status = AsyncLoadingStatus::FAIL;
            return;
        }

        GpuLoadArgs gpuLoadArgs = createArgsForGpuLoad(cpuLoadResult);
        GpuLoadResult gpuLoadResult = gpuLoad(gpuLoadArgs);
        status = AsyncLoadingStatus::GPU_LOAD;
        if (!isGpuLoadSuccessful(gpuLoadResult)) {
            this->status = AsyncLoadingStatus::FAIL;
            return;
        }

        writeCpuGpuLoadResults(cpuLoadResult, gpuLoadResult);
    }
};

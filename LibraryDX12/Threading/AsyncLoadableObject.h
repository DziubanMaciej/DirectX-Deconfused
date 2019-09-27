#pragma once

#include <atomic>

template <typename CpuLoadArgs, typename CpuLoadResult, typename GpuLoadArgs, typename GpuLoadResult>
class AsyncLoadableObject {
protected:
    void cpuGpuLoad(const CpuLoadArgs &args, bool asynchronousLoading) {
        if (asynchronousLoading) {
            auto task = [this, args]() {
                cpuGpuLoadImpl(args);
            };
            ApplicationImpl::getInstance().getBackgroundWorkerController().pushTask(task, this->cpuLoadComplete);
        } else {
            cpuGpuLoadImpl(args);
            this->cpuLoadComplete = true;
        }
    }

    virtual CpuLoadResult cpuLoad(const CpuLoadArgs &args) = 0;
    virtual bool isCpuLoadSuccessful(const CpuLoadResult &result) { return true; }
    virtual GpuLoadArgs createArgsForGpuLoad(const CpuLoadResult &cpuLoadResult) = 0;
    virtual GpuLoadResult gpuLoad(const GpuLoadArgs &args) = 0;
    virtual bool isGpuLoadSuccessful(const GpuLoadResult &result) { return true; }
    virtual void writeCpuGpuLoadResults(CpuLoadResult &cpuLoadResult, GpuLoadResult &gpuLoadResult) = 0;

    void terminateBackgroundProcessing(bool blocking) {
        terminate.store(true);
        if (blocking) {
            while (!cpuLoadComplete.load())
                ;
        }
    }

    bool shouldBackgroundProcessingTerminate() const {
        return terminate.load();
    }

    std::atomic_bool cpuLoadComplete = false;
    std::atomic_bool terminate = false;

private:
    void cpuGpuLoadImpl(const CpuLoadArgs &cpuLoadArgs) {
        CpuLoadResult cpuLoadResult = cpuLoad(cpuLoadArgs);
        if (!isCpuLoadSuccessful(cpuLoadResult)) {
            return; // TODO set some error info
        }

        GpuLoadArgs gpuLoadArgs = createArgsForGpuLoad(cpuLoadResult);
        GpuLoadResult gpuLoadResult = gpuLoad(gpuLoadArgs);
        if (!isGpuLoadSuccessful(gpuLoadResult)) {
            return; // TODO set some error info
        }

        writeCpuGpuLoadResults(cpuLoadResult, gpuLoadResult);
    }
};
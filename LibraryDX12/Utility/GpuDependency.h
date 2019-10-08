#pragma once

#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <vector>

class GpuDependency : DXD::NonCopyable {
public:
    GpuDependency(const CommandQueue &queue, uint64_t fenceValue) : queue(&queue), fenceValue(fenceValue) {}
    GpuDependency(GpuDependency &&other) = default;
    GpuDependency &operator=(GpuDependency &&other) = default;

    bool isComplete() const {
        return queue->isFenceComplete(fenceValue);
    }

    void waitOnCpu() const {
        queue->waitOnCpu(fenceValue);
    }

    void waitOnGpu(CommandQueue &queueToWaitOn) const {
        queueToWaitOn.waitOnGpu(*queue, fenceValue);
    }

private:
    const CommandQueue *queue;
    uint64_t fenceValue;
};

class GpuDependencies : DXD::NonCopyableAndMovable {
public:
    GpuDependencies() = default;

    void add(const CommandQueue &queue, uint64_t fenceValue) {
        dependencies.emplace_back(queue, fenceValue);
    }

    void reset() {
        dependencies.clear();
    }

    bool isComplete() {
        const auto removeIterator = std::remove_if(dependencies.begin(), dependencies.end(), [](const GpuDependency &dependency) {
            return dependency.isComplete();
        });
        dependencies.erase(removeIterator, dependencies.end());
        return dependencies.size() == 0u;
    }

    void waitOnCpu() {
        for (const GpuDependency &dependency : dependencies) {
            dependency.waitOnCpu();
        }
        dependencies.clear();
    }

    void waitOnGpu(CommandQueue &queueToWaitOn) const {
        for (const GpuDependency &dependency : dependencies) {
            dependency.waitOnCpu();
        }
    }

private:
    std::vector<GpuDependency> dependencies;
};

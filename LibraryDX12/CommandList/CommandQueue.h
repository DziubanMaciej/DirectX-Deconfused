#pragma once

#include "CommandList/CommandAllocatorManager.h"
#include "Resource/ResourceUsageTracker.h"
#include "Synchronization/Event.h"
#include "Synchronization/Fence.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <mutex>
#include <vector>

class CommandList;

// TODO: flush and performResourcesDeletion() are not entirely thread safe
class CommandQueue : DXD::NonCopyableAndMovable {
public:
    CommandQueue(ID3D12DevicePtr device, D3D12_COMMAND_LIST_TYPE type);
    ~CommandQueue();

    auto getCommandQueue() { return commandQueue; }
    auto &getFence() { return fence; }
    auto &getCommandAllocatorManager() { return commandAllocatorManager; }

    uint64_t executeCommandListsAndSignal(std::vector<CommandList *> &commandLists);
    void performResourcesDeletion();

    void flush();

private:
    uint64_t executeCommandListsAndSignal(std::vector<ID3D12CommandList *> &commandListsPtrs);
    static ID3D12CommandQueuePtr createCommandQueue(ID3D12DevicePtr &device, D3D12_COMMAND_LIST_TYPE type);

    std::mutex lock;

    ID3D12DevicePtr device;
    ID3D12CommandQueuePtr commandQueue;
    D3D12_COMMAND_LIST_TYPE type;
    CommandAllocatorManager commandAllocatorManager;
    ResourceUsageTracker resourceUsageTracker;
    Fence fence;
};

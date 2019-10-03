#pragma once

#include "CommandList/CommandAllocatorController.h"
#include "Resource/ResourceUsageTracker.h"
#include "Synchronization/Event.h"
#include "Synchronization/Fence.h"

#include <DXD/ExternalHeadersWrappers/d3d12.h>
#include <mutex>
#include <vector>

class CommandList;

class CommandQueue : DXD::NonCopyableAndMovable {
public:
    CommandQueue(ID3D12DevicePtr device, D3D12_COMMAND_LIST_TYPE type);
    ~CommandQueue();

    auto getCommandQueue() { return commandQueue; }
    auto &getFence() { return fence; }
    auto &getCommandAllocatorController() { return commandAllocatorController; }

    uint64_t executeCommandListsAndSignal(std::vector<CommandList *> &commandLists);
    uint64_t executeCommandListAndSignal(CommandList &commandList);
    void performResourcesDeletion(bool lockQueue);

    void flush(bool lockQueue);
    std::unique_lock<std::mutex> getLock(bool lockQueue);

private:
    static ID3D12CommandQueuePtr createCommandQueue(ID3D12DevicePtr &device, D3D12_COMMAND_LIST_TYPE type);

    std::mutex lock;

    ID3D12DevicePtr device;
    ID3D12CommandQueuePtr commandQueue;
    D3D12_COMMAND_LIST_TYPE type;
    CommandAllocatorController commandAllocatorController;
    ResourceUsageTracker resourceUsageTracker;
    Fence fence;
};

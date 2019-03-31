#pragma once

#include "Source/Event.h"
#include "Source/Fence.h"
#include "DXD/ExternalHeadersWrappers/d3d12.h"

#include <vector>
#include <deque>

/*
    CommandAllocator cannot be reused until CommandList allocated by it has finished executing.
    This class maintains a queue of allocators of one given type and fences indicating they are complete.

    CommandList can be reused immediately after executeCommandLists() call, queue of these objects is also
    maintained but we do not need to track any fences.

    Usage patterns of this class is as follows:
    - getCommandAllocator()
    - getCommandList()
    - record to command list
    - execute command list
    - registerAllocatorAndList(allocator, list)
*/
class CommandAllocatorManager : DXD::NonCopyableAndMovable {
    struct CommandAllocatorEntry {
        ID3D12CommandAllocatorPtr commandAllocator;
        uint64_t lastFence;
    };

public:
    CommandAllocatorManager(ID3D12DevicePtr device, Fence &fence, D3D12_COMMAND_LIST_TYPE type);
    virtual ~CommandAllocatorManager() = default;

    // Get methods take reusable objects from queues or create new ones if there is nothing to reuse. They do not add to queues.
    ID3D12CommandAllocatorPtr getCommandAllocator();
    ID3D12GraphicsCommandListPtr getCommandList(ID3D12CommandAllocatorPtr commandAllocator, ID3D12PipelineState *initialPipelineState);

    // Allocator and list should be registered (added to queues) for later reuse after call to executeCommandLists().
    void registerAllocatorAndList(ID3D12CommandAllocatorPtr commandAllocator, ID3D12GraphicsCommandListPtr commandList, uint64_t fence);

protected:
    ID3D12CommandAllocatorPtr createCommandAllocator();
    ID3D12GraphicsCommandListPtr createCommandList(ID3D12CommandAllocatorPtr commandAllocator, ID3D12PipelineState *initialPipelineState);
    void validateCommandAllocators();
    void validateCommandLists();

    ID3D12DevicePtr device;
    const Fence &fence;
    const D3D12_COMMAND_LIST_TYPE type;

    std::deque<CommandAllocatorEntry> commandAllocators;
    std::deque<ID3D12GraphicsCommandListPtr> commandLists;
};

class CommandQueue : DXD::NonCopyableAndMovable {
public:
    CommandQueue(ID3D12DevicePtr device, D3D12_COMMAND_LIST_TYPE type);

    auto getCommandQueue() { return commandQueue; }
    auto &getFence() { return fence; }

    uint64_t executeCommandListsAndSignal(std::vector<ID3D12CommandList *> &commandListsPtrs);
    uint64_t executeCommandListsAndSignal(std::vector<ID3D12GraphicsCommandListPtr> &commandListComPtrs);

    void flush(Event &fenceEvent);
    void wait(Event &fenceEvent);

private:
    static ID3D12CommandQueuePtr createCommandQueue(ID3D12DevicePtr &device, D3D12_COMMAND_LIST_TYPE type);

    ID3D12DevicePtr device;
    ID3D12CommandQueuePtr commandQueue;
    D3D12_COMMAND_LIST_TYPE type;
    CommandAllocatorManager commandAllocatorManager;
    Fence fence;
    uint64_t lastFence;
};

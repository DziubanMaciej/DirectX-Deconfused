#pragma once

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <deque>

class Fence;

/// \brief Class encapsulating pooled CommandAllocators and CommandLists for easier reuse
///
/// CommandAllocator cannot be reused until CommandList allocated by it has finished executing.
/// This class maintains a queue of CommandAllocatorEntries. An entry is commandAllocator paired with fence
/// value indicating the command being completed. CommandList can be reused immediately after executeCommandLists()
/// call, queue of these objects is also maintained but no fence are tracked. To minimize creations of CommandAllocators and
/// CommandLists, registerAllocatorAndList(ID3D12CommandAllocatorPtr, ID3D12GraphicsCommandListPtr, uint64_t) should be called
/// as soon as the CommandList is scheduled for execution to the CommandQueue.
///
/// This class should be directly used only by CommandList class
class CommandAllocatorManager : DXD::NonCopyableAndMovable {
    struct CommandAllocatorEntry {
        ID3D12CommandAllocatorPtr commandAllocator;
        uint64_t lastFence;
    };

public:
    CommandAllocatorManager(ID3D12DevicePtr device, Fence &fence, D3D12_COMMAND_LIST_TYPE type);
    virtual ~CommandAllocatorManager() = default;

    /// Retrieves command allocator that was registered to this manager
    /// (see registerAllocatorAndList(ID3D12CommandAllocatorPtr, ID3D12GraphicsCommandListPtr, uint64_t)).
    /// and is done executing or creates new one if there is nothing for reuse. This method assumes the
    /// allocators are registered in a non-decreasing order (ordered by their fenceValues) and checks only
    /// the first entry of the queue.
    /// \return valid DX12 command allocator
    ID3D12CommandAllocatorPtr retieveCommandAllocator();

    /// Retrieves command list that was registered to this manager
    /// (see registerAllocatorAndList(ID3D12CommandAllocatorPtr, ID3D12GraphicsCommandListPtr, uint64_t))
    /// or created a new one if there is nothing for reuse.
    /// \param commandAllocator Object returned by retrieveCommandAllocator()
    /// \param initialPipelineState Optional pointer to pipeline state object for the command list to be initialized with
    /// \return valid DX12 command list
    ID3D12GraphicsCommandListPtr retrieveCommandList(ID3D12CommandAllocatorPtr commandAllocator, ID3D12PipelineState *initialPipelineState);

    /// Registers command allocator and command list for later reuse. Allocators are paired with the fence value and can be reused
    /// after this value is reached (meaning commands have been completed). CommandAllocators should by registered in the
    /// non-decreasing order of fenceValues (see retrieveCommandAllocator()). Command lists are simply added to queue and are able
    /// to be reused immediately after call to this function.
    /// \param commandAllocator A valid command allocator
    /// \param commandList List created with the allocator specified. List have to be closed
    /// \param fenceValue Fence Valueindicating completion of commands in the commandList
    void registerAllocatorAndList(ID3D12CommandAllocatorPtr commandAllocator, ID3D12GraphicsCommandListPtr commandList, uint64_t fenceValue);

    auto getDevice() const { return device; }
    auto getType() const { return type; }

protected:
    ID3D12CommandAllocatorPtr createCommandAllocator();
    void validateCommandAllocators();
    void validateCommandLists();

    ID3D12DevicePtr device;
    const Fence &fence;
    const D3D12_COMMAND_LIST_TYPE type;

    std::deque<CommandAllocatorEntry> commandAllocators;
    std::deque<ID3D12GraphicsCommandListPtr> commandLists;
};

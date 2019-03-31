#include "SceneImpl.h"
#include "Source/ApplicationImpl.h"
#include "Source/CommandQueue.h"
#include "Source/WindowImpl.h"
#include "Utility/ThrowIfFailed.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"

namespace DXD {
std::unique_ptr<Scene> Scene::create() {
    return std::unique_ptr<Scene>{new SceneImpl()};
}
} // namespace DXD

SceneImpl::SceneImpl() {
}

void SceneImpl::setBackgroundColor(float r, float g, float b) {
    backgroundColor[0] = r;
    backgroundColor[1] = g;
    backgroundColor[2] = b;
}

void SceneImpl::render(ApplicationImpl &application, SwapChain &swapChain) {
    auto &commandQueue = application.getDirectCommandQueue();
    auto &commandAllocatorManager = commandQueue.getCommandAllocatorManager();
    const auto commandAllocator = commandAllocatorManager.getCommandAllocator();
    const auto commandList = commandAllocatorManager.getCommandList(commandAllocator, nullptr);
    const auto &backBuffer = swapChain.getCurrentBackBuffer();

    // Transition to RENDER_TARGET
    const auto renderTargetTransition = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(),
                                                                             D3D12_RESOURCE_STATE_PRESENT,
                                                                             D3D12_RESOURCE_STATE_RENDER_TARGET);
    commandList->ResourceBarrier(1, &renderTargetTransition);

    // Render (clear color)
    const FLOAT clearColor[] = {0.0f, 0.8f, 0.8f};
    commandList->ClearRenderTargetView(swapChain.getCurrentBackBufferDescriptor(), clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(swapChain.getDepthStencilBufferDescriptor(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

    // Transition to PRESENT
    CD3DX12_RESOURCE_BARRIER presentTransition = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer.Get(),
                                                                                      D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                                      D3D12_RESOURCE_STATE_PRESENT);
    commandList->ResourceBarrier(1, &presentTransition);

    // Close command list
    throwIfFailed(commandList->Close());

    // Execute and register obtained allocator and lists to the manager
    std::vector<ID3D12GraphicsCommandListPtr> commandLists{commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);
    commandAllocatorManager.registerAllocatorAndList(commandAllocator, commandList, fenceValue);

    // Present (swap back buffers) and wait for next frame's fence
    swapChain.present(fenceValue);
    commandQueue.getFence().wait(swapChain.getFenceValueForCurrentBackBuffer());
}

#include "SceneImpl.h"
#include "Source/ApplicationImpl.h"
#include "Source/CommandList.h"
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
    CommandList commandList{commandQueue.getCommandAllocatorManager(), nullptr};
    const auto &backBuffer = swapChain.getCurrentBackBuffer();

    // Transition to RENDER_TARGET
    commandList.transitionBarrierSingle(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // Render (clear color)
    commandList.clearRenderTargetView(swapChain.getCurrentBackBufferDescriptor(), backgroundColor);
    commandList.clearDepthStencilView(swapChain.getDepthStencilBufferDescriptor(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

    // Transition to PRESENT
    commandList.transitionBarrierSingle(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    // Close command list
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);

    // Present (swap back buffers) and wait for next frame's fence
    swapChain.present(fenceValue);
    commandQueue.getFence().wait(swapChain.getFenceValueForCurrentBackBuffer());
}

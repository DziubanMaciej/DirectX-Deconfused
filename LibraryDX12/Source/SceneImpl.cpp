#include "SceneImpl.h"
#include "Source/ApplicationImpl.h"
#include "Source/CommandList.h"
#include "Source/CommandQueue.h"
#include "Source/WindowImpl.h"
#include "Utility/ThrowIfFailed.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <algorithm>
#include <cassert>

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

void SceneImpl::addObject(DXD::Object &object) {
    objects.insert(static_cast<ObjectImpl *>(&object));
}

bool SceneImpl::removeObject(DXD::Object &object) {
    const auto elementsRemoved = objects.erase(static_cast<ObjectImpl *>(&object));
    assert(elementsRemoved < 2u);
    return elementsRemoved == 1;
}

void SceneImpl::setCamera(DXD::Camera &camera) {
    this->camera = static_cast<CameraImpl *>(&camera);
}

DXD::Camera *SceneImpl::getCamera() {
    return camera;
}

void SceneImpl::render(ApplicationImpl &application, SwapChain &swapChain) {
    auto &commandQueue = application.getDirectCommandQueue();
    CommandList commandList{commandQueue.getCommandAllocatorManager(), nullptr};
    const auto &backBuffer = swapChain.getCurrentBackBuffer();
    commandQueue.performResourcesDeletion();

    // Transition to RENDER_TARGET
    commandList.transitionBarrierSingle(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    // Render (clear color)
    commandList.clearRenderTargetView(swapChain.getCurrentBackBufferDescriptor(), backgroundColor);
    commandList.clearDepthStencilView(swapChain.getDepthStencilBufferDescriptor(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0);

    // Pipeline state
    commandList.setPipelineState(application.getPipelineStateController().getPipelineState(PipelineStateController::Identifier::PIPELINE_STATE_DEFAULT));
    commandList.setGraphicsRootSignature(application.getPipelineStateController().getRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_DEFAULT));

    // View projection matrix
    float aspectRatio = (float)swapChain.getWidth() / swapChain.getHeight();
    camera->setAspectRatio(aspectRatio);
    const XMMATRIX viewMatrix = camera->getViewMatrix();
    const XMMATRIX projectionMatrix = camera->getProjectionMatrix();
    const XMMATRIX vpMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);

    for (ObjectImpl *object : objects) {
        MeshImpl &mesh = *object->getMesh();

        commandList.IASetVertexBuffer(mesh.getVertexBufferView(), mesh.getVertexBuffer());
        commandList.IASetIndexBuffer(mesh.getIndexBufferView(), mesh.getIndexBuffer());
        commandList.IASetPrimitiveTopologyTriangleList();

        CD3DX12_VIEWPORT viewport(0.0f, 0.0f, (float)swapChain.getWidth(), (float)swapChain.getHeight());
        CD3DX12_RECT scissorRect(0, 0, LONG_MAX, LONG_MAX);
        commandList.RSSetScissorRect(scissorRect);
        commandList.RSSetViewport(viewport);

        commandList.OMSetRenderTarget(swapChain.getCurrentBackBufferDescriptor(), swapChain.getCurrentBackBuffer(),
                                      swapChain.getDepthStencilBufferDescriptor(), swapChain.getDepthStencilBuffer());

        const XMMATRIX modelMatrix = object->getModelMatrix();
        const XMMATRIX mvpMatrix = XMMatrixMultiply(modelMatrix, vpMatrix);
        commandList.setGraphicsRoot32BitConstant(0, mvpMatrix);

        commandList.drawIndexed(static_cast<UINT>(mesh.getIndicesCount()));
    }

    // Transition to PRESENT
    commandList.transitionBarrierSingle(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    // Close command list
    commandList.close();

    // Execute and register obtained allocator and lists to the manager
    std::vector<CommandList *> commandLists{&commandList};
    const uint64_t fenceValue = commandQueue.executeCommandListsAndSignal(commandLists);

    // Present (swap back buffers) and wait for next frame's fence
    swapChain.present(fenceValue);
    commandQueue.getFence().waitOnCpu(swapChain.getFenceValueForCurrentBackBuffer());
}

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

    commandList.setPipelineState(application.getPipelineStateController().getPipelineState(PipelineStateController::Identifier::PIPELINE_STATE_DEFAULT));
    commandList.setGraphicsRootSignature(application.getPipelineStateController().getRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_DEFAULT));

    MeshImpl &mesh = *(*objects.begin())->getMesh();

    commandList.IASetVertexBuffer(mesh.getVertexBufferView(), mesh.getVertexBuffer());
    commandList.IASetIndexBuffer(mesh.getIndexBufferView(), mesh.getIndexBuffer());
    commandList.IASetPrimitiveTopologyTriangleList();

    CD3DX12_VIEWPORT viewport(0.0f, 0.0f, 300.f, 300.f);
    CD3DX12_RECT scissorRect(0, 0, LONG_MAX, LONG_MAX);
    commandList.RSSetScissorRect(scissorRect);
    commandList.RSSetViewport(viewport);

    commandList.OMSetRenderTarget(swapChain.getCurrentBackBufferDescriptor(), swapChain.getCurrentBackBuffer(),
                                  swapChain.getDepthStencilBufferDescriptor(), swapChain.getDepthStencilBuffer());

    static float roll = 0, pitch = 0, yaw = 0;
    roll += 0.05f;
    pitch += 0.1f;

    XMMATRIX modelMatrix = XMMatrixTransformation(
        XMVECTOR{0, 0, 0}, XMQuaternionIdentity(), XMVECTOR{1, 1, 1},          // scaling
        XMVECTOR{0, 0, 0}, XMQuaternionRotationRollPitchYaw(roll, pitch, yaw), // rotation
        XMVECTOR{0, 0, 0}                                                      // translation
    );
    const XMVECTOR eyePosition = XMVectorSet(0, 0, -50, 1);
    const XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
    const XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
    XMMATRIX viewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

    XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45), 1, 0.1f, 100.0f);

    XMMATRIX mvpMatrix = XMMatrixMultiply(modelMatrix, viewMatrix);
    mvpMatrix = XMMatrixMultiply(mvpMatrix, projectionMatrix);
    commandList.setGraphicsRoot32BitConstant(0, mvpMatrix);

    commandList.drawIndexed(static_cast<UINT>(mesh.getIndicesCount()));

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

#pragma once

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/d3d12.h>

class CommandList;
class ConstantBuffer;
class RenderData;
class Resource;
class SceneImpl;
class SwapChain;

class DeferredShadingRenderer : DXD::NonCopyableAndMovable {
public:
    DeferredShadingRenderer(SwapChain &swapChain, RenderData &renderData, SceneImpl &scene, bool shadowsEnabled);

    void renderGBuffers(CommandList &commandList);
    void renderLighting(CommandList &commandList, Resource &output);

private:
    D3D12_CPU_DESCRIPTOR_HANDLE uploadLightingConstantBuffer(ConstantBuffer &lightingConstantBuffer);

    SwapChain &swapChain;
    RenderData &renderData;
    SceneImpl &scene;
    const bool shadowsEnabled;
};

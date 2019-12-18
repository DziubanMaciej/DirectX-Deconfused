#pragma once

#include "DXD/Utility/NonCopyableAndMovable.h"

class CommandList;
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
    SwapChain &swapChain;
    RenderData &renderData;
    SceneImpl &scene;
    const bool shadowsEnabled;
};
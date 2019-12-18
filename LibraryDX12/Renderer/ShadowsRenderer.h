#pragma once

#include "DXD/Utility/NonCopyableAndMovable.h"

class RenderData;
class SceneImpl;
class SwapChain;
class CommandList;

class ShadowsRenderer : public DXD::NonCopyableAndMovable {
public:
    ShadowsRenderer(SwapChain &swapChain, RenderData &renderData, SceneImpl &scene);

    void renderShadowMaps(CommandList &commandList);
    bool isEnabled() const { return enabled; }

private:
    SwapChain &swapChain;
    RenderData &renderData;
    SceneImpl &scene;
    const bool enabled;
};

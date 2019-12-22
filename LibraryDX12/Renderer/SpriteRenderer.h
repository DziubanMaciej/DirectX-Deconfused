#pragma once

#include "DXD/Utility/NonCopyableAndMovable.h"

class CommandList;
class RenderData;
class Resource;
class SceneImpl;
class SpriteImpl;
class SwapChain;

class SpriteRenderer : DXD::NonCopyableAndMovable {
public:
    SpriteRenderer(SwapChain &swapChain, RenderData &renderData, SceneImpl &scene);

    void renderSprites(CommandList &commandList, Resource &output);

private:
    void renderSprite(CommandList &commandList, Resource &output, SpriteImpl *sprite);

    SwapChain &swapChain;
    RenderData &renderData;
    SceneImpl &scene;
};

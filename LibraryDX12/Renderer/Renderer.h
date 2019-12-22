#pragma once

#include "Renderer/DeferredShadingRenderer.h"
#include "Renderer/PostProcessRenderer.h"
#include "Renderer/ShadowsRenderer.h"
#include "Renderer/SpriteRenderer.h"
#include "Renderer/TextRenderer.h"

#include <DXD/ExternalHeadersWrappers/DirectXMath.h>
#include <vector>

class SwapChain;
class RenderData;
class CommandList;
class Resource;
class PostProcessImpl;
struct AlternatingResources;
class SpriteImpl;
class SceneImpl;

class Renderer {
public:
    Renderer(SwapChain &swapChain, RenderData &renderData, SceneImpl &scene);
    void render();

private:
    // Render methods
    void renderSSAO(CommandList &commandList);
    void renderSSRandMerge(CommandList &commandList, Resource &input, Resource &output);
    void renderFog(CommandList &commandList, Resource &input, Resource &output);
    void renderDof(CommandList &commandList, Resource &input, Resource &output);
    void copyToBackBuffer(CommandList &commandList, Resource &source);

    // Data
    SwapChain &swapChain;
    RenderData &renderData;
    SceneImpl &scene;

    // Sub-renderers
    ShadowsRenderer shadowsRenderer;
    DeferredShadingRenderer deferredShadingRenderer;
    PostProcessRenderer postProcessRenderer;
    SpriteRenderer spriteRenderer;
    TextRenderer textRenderer;
};

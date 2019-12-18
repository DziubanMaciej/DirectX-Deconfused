#pragma once

#include "Renderer/ShadowsRenderer.h"
#include "Renderer/DeferredShadingRenderer.h"

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
    void renderPostProcess(PostProcessImpl &postProcess, CommandList &commandList, Resource *optionalInput,
                           AlternatingResources &alternatingResources, float screenWidth, float screenHeight);
    void renderPostProcesses(CommandList &commandList, const std::vector<PostProcessImpl *> &postProcesses,
                             AlternatingResources &alternatingResources, size_t enabledPostProcessesCount,
                             float screenWidth, float screenHeight);
    void renderBloom(CommandList &commandList, Resource &input, Resource &output);
    void renderD2DTexts();
    void renderSprite(CommandList &commandList, SpriteImpl *sprite);

    // Helpers
    size_t getEnabledPostProcessesCount() const;
    static void getSourceAndDestinationForPostProcess(AlternatingResources &alternatingResources, Resource *optionalInput,
                                                      Resource *&outSource, Resource *&outDestination);
    static void prepareSourceAndDestinationForPostProcess(CommandList &commandList, Resource &source, Resource &destination, bool compute = false);

    // Data
    SwapChain &swapChain;
    RenderData &renderData;
    SceneImpl &scene;

    // Sub-renderers
    ShadowsRenderer shadowsRenderer;
    DeferredShadingRenderer deferredShadingRenderer;
};

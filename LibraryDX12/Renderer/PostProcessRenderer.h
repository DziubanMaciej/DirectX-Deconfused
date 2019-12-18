#pragma once

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <vector>

struct AlternatingResources;
class CommandList;
class PostProcessImpl;
class RenderData;
class Resource;
class SceneImpl;
class SwapChain;

class PostProcessRenderer : DXD::NonCopyableAndMovable {
public:
    PostProcessRenderer(RenderData &renderData, const std::vector<PostProcessImpl *> &postProcesses, float screenWidth, float screenHeight);

    void renderPostProcesses(CommandList &commandList, AlternatingResources &alternatingResources,
                             size_t enabledPostProcessesCount, float screenWidth, float screenHeight);
    void renderPostProcess(PostProcessImpl &postProcess, CommandList &commandList, Resource *optionalInput,
                           AlternatingResources &alternatingResources, float screenWidth, float screenHeight);
    void renderBloom(CommandList &commandList, Resource &input, Resource &output);

    size_t getEnabledPostProcessesCount() const;

private:
    static void getSourceAndDestinationForPostProcess(AlternatingResources &alternatingResources, Resource *optionalInput,
                                                      Resource *&outSource, Resource *&outDestination);
    static void prepareSourceAndDestinationForPostProcess(CommandList &commandList, Resource &source, Resource &destination, bool compute = false);

    RenderData &renderData;
    const std::vector<PostProcessImpl *> &postProcesses;
    const float screenWidth;
    const float screenHeight;
};

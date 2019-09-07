#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "Descriptor/DescriptorController.h"
#include "Resource/ConstantBuffer.h"
#include "Scene/PostProcessImpl.h"
#include "Utility/AlternatingResources.h"

#include <vector>

struct PostProcessRenderTargets : AlternatingResources {
    using AlternatingResources::AlternatingResources;
    void resize(int width, int height) override;
};

class RenderData {
public:
    RenderData(ID3D12DevicePtr &device, DescriptorController &descriptorController, int width, int height);
    void resize(int width, int height);

    // Getters
    PostProcessRenderTargets &getPostProcessRenderTargets() { return postProcessRenderTargets; }
    Resource &getShadowMap(int i) { return *shadowMap[i]; }
    Resource &getDepthStencilBuffer() { return *depthStencilBuffer; };
    Resource &getBloomMap() { return *bloomMap; }
    auto &getPostProcessesForBloom() { return postProcessesForBloom; }

private:
    // Base objects
    ID3D12DevicePtr device = {};

    // Resources
    PostProcessRenderTargets postProcessRenderTargets;
    std::unique_ptr<Resource> bloomMap;
    std::unique_ptr<Resource> shadowMap[8] = {};
    std::unique_ptr<Resource> depthStencilBuffer = {};

    // Misc
    std::unique_ptr<DXD::PostProcess> postProcessForBloom;
    std::vector<PostProcessImpl *> postProcessesForBloom = {};
};

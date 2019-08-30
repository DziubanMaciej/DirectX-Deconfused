#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "Descriptor/DescriptorController.h"
#include "Resource/ConstantBuffer.h"
#include "Utility/AlternatingResources.h"

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

private:
    // Base objects
    ID3D12DevicePtr device = {};

    // Resources
    PostProcessRenderTargets postProcessRenderTargets;
    std::unique_ptr<Resource> shadowMap[8] = {};
    std::unique_ptr<Resource> depthStencilBuffer = {};
};

#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "Descriptor/DescriptorController.h"
#include "Resource/ConstantBuffer.h"
#include "Resource/RenderTarget.h"

struct PostProcessRenderTargets {
    PostProcessRenderTargets(ID3D12DevicePtr &device, DescriptorController &descriptorController);
    void resize(int width, int height);

    RenderTarget &getSource() { return *resources[sourceResourceIndex]; }
    RenderTarget &getDestination() { return *resources[1 - sourceResourceIndex]; }

    void swapResources() {
        sourceResourceIndex = 1 - sourceResourceIndex;
    }

private:
    ID3D12DevicePtr device = {};
    int sourceResourceIndex = 0;
    std::unique_ptr<RenderTarget> resources[2] = {};
};

class RenderData {
public:
    RenderData(ID3D12DevicePtr &device, DescriptorController &descriptorController, int width, int height);
    void resize(int width, int height);

    // Getters
    PostProcessRenderTargets &getPostProcessRenderTargets() { return postProcessRenderTargets; }

    Resource &getShadowMap(int i) { return *shadowMap[i]; }
    const DescriptorAllocation &getShadowMapDsvDescriptors() const { return shadowMapDsvDescriptors; }
    const DescriptorAllocation &getShadowMapSrvDescriptors() const { return shadowMapSrvDescriptors; }

    Resource &getDepthStencilBuffer() { return *depthStencilBuffer; };
    const DescriptorAllocation &getDepthStencilBufferDescriptor() const { return dsvDescriptor; }

private:
    // Base objects
    ID3D12DevicePtr device = {};

    // Post process render targets
    PostProcessRenderTargets postProcessRenderTargets;

    // Shadow maps
    std::unique_ptr<Resource> shadowMap[8] = {};
    DescriptorAllocation shadowMapDsvDescriptors = {};
    DescriptorAllocation shadowMapSrvDescriptors = {};

    // Depth stencil buffer
    std::unique_ptr<Resource> depthStencilBuffer = {};
    DescriptorAllocation dsvDescriptor = {};
};

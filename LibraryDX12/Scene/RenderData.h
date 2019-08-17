#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "Descriptor/DescriptorManager.h"
#include "Resource/Resource.h"

class RenderData {
public:
    RenderData(ID3D12DevicePtr &device, DescriptorManager &descriptorManager, int width, int height);
    void resize(int width, int height);

    // Getters
    const DescriptorAllocation &getPostProcessSrvDescriptor() const { return postProcessSrvDescriptor; }
    const DescriptorAllocation &getPostProcessRtvDescriptor() const { return postProcessRtvDescriptor; }
    const DescriptorAllocation &getShadowMapDsvDescriptors() const { return shadowMapDsvDescriptors; }
    const DescriptorAllocation &getShadowMapSrvDescriptors() const { return shadowMapSrvDescriptors; }
    Resource &getPostProcessRenderTarget() { return *postProcessRenderTarget; }
    Resource &getShadowMap(int i) { return *shadowMap[i]; }

private:
    // Base objects
    ID3D12DevicePtr device = {};

    // Descriptors
    DescriptorAllocation postProcessSrvDescriptor = {};
    DescriptorAllocation postProcessRtvDescriptor = {};
    DescriptorAllocation shadowMapDsvDescriptors = {};
    DescriptorAllocation shadowMapSrvDescriptors = {};

    // Render Targets
    std::unique_ptr<Resource> postProcessRenderTarget = {};

    // Shadow Maps
    std::unique_ptr<Resource> shadowMap[8] = {};
};

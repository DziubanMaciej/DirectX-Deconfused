#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "Descriptor/DescriptorController.h"
#include "Resource/Resource.h"
#include "Resource/ConstantBuffer.h"

class RenderData {
public:
    RenderData(ID3D12DevicePtr &device, DescriptorController &descriptorController, int width, int height);
    void resize(int width, int height);

    // Getters
    Resource &getPostProcessRenderTarget() { return *postProcessRenderTarget; }
    const DescriptorAllocation &getPostProcessSrvDescriptor() const { return postProcessSrvDescriptor; }
    const DescriptorAllocation &getPostProcessRtvDescriptor() const { return postProcessRtvDescriptor; }
    ConstantBuffer &getPostProcessConvolutionCB() { return postProcessConvolutionCb; }

    Resource &getShadowMap(int i) { return *shadowMap[i]; }
    const DescriptorAllocation &getShadowMapDsvDescriptors() const { return shadowMapDsvDescriptors; }
    const DescriptorAllocation &getShadowMapSrvDescriptors() const { return shadowMapSrvDescriptors; }

    Resource &getDepthStencilBuffer() { return *depthStencilBuffer; };
    const DescriptorAllocation &getDepthStencilBufferDescriptor() const { return dsvDescriptor; }

private:
    // Base objects
    ID3D12DevicePtr device = {};

    // Post process render target
    std::unique_ptr<Resource> postProcessRenderTarget = {};
    DescriptorAllocation postProcessSrvDescriptor = {};
    DescriptorAllocation postProcessRtvDescriptor = {};
    ConstantBuffer postProcessConvolutionCb;

    // Shadow maps
    std::unique_ptr<Resource> shadowMap[8] = {};
    DescriptorAllocation shadowMapDsvDescriptors = {};
    DescriptorAllocation shadowMapSrvDescriptors = {};

    // Depth stencil buffer
    std::unique_ptr<Resource> depthStencilBuffer = {};
    DescriptorAllocation dsvDescriptor = {};
};

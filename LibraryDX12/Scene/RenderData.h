#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "Descriptor/DescriptorController.h"
#include "Resource/ConstantBuffer.h"
#include "Resource/Resource.h"

struct PostProcessRenderTargets {
    PostProcessRenderTargets(ID3D12DevicePtr &device, DescriptorController &descriptorController);
    void resize(int width, int height);

    Resource &getSource() { return *resources[sourceResourceIndex]; }
    Resource &getDestination() { return *resources[destinationResourceIndex]; }
    D3D12_CPU_DESCRIPTOR_HANDLE getSourceSrv() { return srvDescriptors.getCpuHandle(sourceResourceIndex); }
    D3D12_CPU_DESCRIPTOR_HANDLE getDestinationSrv() { return srvDescriptors.getCpuHandle(destinationResourceIndex); }
    D3D12_CPU_DESCRIPTOR_HANDLE getSourceRtv() { return rtvDescriptors.getCpuHandle(sourceResourceIndex); }
    D3D12_CPU_DESCRIPTOR_HANDLE getDestinationRtv() { return rtvDescriptors.getCpuHandle(destinationResourceIndex); }

    void swapResources() {
        sourceResourceIndex = 1 - sourceResourceIndex;
        destinationResourceIndex = 1 - destinationResourceIndex;
    }

private:
    ID3D12DevicePtr device = {};
    int sourceResourceIndex = 0;
    int destinationResourceIndex = 1;
    std::unique_ptr<Resource> resources[2] = {};
    DescriptorAllocation srvDescriptors;
    DescriptorAllocation rtvDescriptors;
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

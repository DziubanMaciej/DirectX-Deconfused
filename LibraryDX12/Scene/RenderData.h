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
    ~RenderData();
    void resize(int width, int height);

    // Getters
    PostProcessRenderTargets &getPostProcessRenderTargets() { return postProcessRenderTargets; }
    Resource &getShadowMap(int i) { return *shadowMap[i]; }
    UINT getShadowMapSize() { return shadowMapSize; }
    Resource &getDepthStencilBuffer() { return *depthStencilBuffer; };
    Resource &getLightingOutput() { return *lightingOutput; }
    Resource &getSsaoMap() { return *ssaoMap; }
    Resource &getBloomMap() { return *bloomMap; }
    Resource &getGBufferAlbedo() { return *gBufferAlbedo; }
    Resource &getGBufferNormal() { return *gBufferNormal; }
    Resource &getGBufferSpecular() { return *gBufferSpecular; }
    PostProcessImpl &getPostProcessForBloom() { return *static_cast<PostProcessImpl*>(postProcessForBloom.get()); }

private:
    void createShadowMaps(unsigned int shadowsQuality);

    // Base objects
    ID3D12DevicePtr device = {};

    // Resources
    PostProcessRenderTargets postProcessRenderTargets;
    std::unique_ptr<Resource> ssaoMap;
    std::unique_ptr<Resource> gBufferAlbedo;
    std::unique_ptr<Resource> gBufferNormal;
    std::unique_ptr<Resource> gBufferSpecular;
    std::unique_ptr<Resource> bloomMap;
    std::unique_ptr<Resource> lightingOutput;
    std::unique_ptr<Resource> shadowMap[8] = {};
    std::unique_ptr<Resource> depthStencilBuffer = {};

    // Numerical state
    UINT shadowMapSize{};

    // Misc
    std::unique_ptr<DXD::PostProcess> postProcessForBloom;
};

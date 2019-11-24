#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "Descriptor/DescriptorController.h"
#include "Resource/ConstantBuffer.h"
#include "Scene/PostProcessImpl.h"
#include "Utility/AlternatingResources.h"

#include <vector>

class RenderData {
public:
    struct PostProcessRenderTargets : AlternatingResources {
        using AlternatingResources::AlternatingResources;
        void resize(int width, int height) override;
    };

    RenderData(int width, int height);
    ~RenderData();
    void resize(int width, int height);

    // Getters
    AlternatingResources &getPostProcessRenderTargets() { return postProcessRenderTargets; }
    Resource &getShadowMap(int i) { return *shadowMap[i]; }
    UINT getShadowMapSize() { return shadowMapSize; }
    Resource &getDepthStencilBuffer() { return *depthStencilBuffer; };
    Resource &getLightingOutput() { return *lightingOutput; }
    Resource &getSsaoMap() { return *ssaoMap; }
    Resource &getSsrMap() { return *ssrMap; }
    Resource &getSsrBlurredMap() { return *ssrBlurredMap; }
    Resource &getBloomMap() { return *bloomMap; }
    Resource &getPreFogBuffer() { return *preFogBuffer; }
    Resource &getPreDofBuffer() { return *preDofBuffer; }
    Resource &getGBufferAlbedo() { return *gBufferAlbedo; }
    Resource &getGBufferNormal() { return *gBufferNormal; }
    Resource &getGBufferSpecular() { return *gBufferSpecular; }
    PostProcessImpl &getPostProcessForBloom() { return *static_cast<PostProcessImpl *>(postProcessForBloom.get()); }
    VertexBuffer &getFullscreenVB() { return *fullscreenVB; }
    ConstantBuffer &getLightingConstantBuffer() { return lightingConstantBuffer; }

private:
    // Buffer creations
    void createShadowMaps(unsigned int shadowsQuality);
    void createGBuffers(int width, int height);
    void createBloomBuffers(int width, int height);
    void createSsaoBuffers(int width, int height);
    void createSsrBuffers(int width, int height);
    void createLightingOutputBuffers(int width, int height);
    void createPreFogBuffer(int width, int height);
    void createPreDofBuffer(int width, int height);
    void createDepthBuffer(int width, int height);

    // Helpers
    static D3D12_RESOURCE_DESC getBaseDescForFullscreenTexture(int width, int height);

    // Base objects
    ID3D12DevicePtr device = {};

    // Resources
    PostProcessRenderTargets postProcessRenderTargets;
    std::unique_ptr<Resource> ssaoMap;
    std::unique_ptr<Resource> ssrMap;
    std::unique_ptr<Resource> ssrBlurredMap;
    std::unique_ptr<Resource> gBufferAlbedo;
    std::unique_ptr<Resource> gBufferNormal;
    std::unique_ptr<Resource> gBufferSpecular;
    std::unique_ptr<Resource> bloomMap;
    std::unique_ptr<Resource> lightingOutput;
    std::unique_ptr<Resource> preFogBuffer;
    std::unique_ptr<Resource> preDofBuffer;
    std::unique_ptr<Resource> shadowMap[8] = {};
    std::unique_ptr<Resource> depthStencilBuffer = {};
    std::unique_ptr<VertexBuffer> fullscreenVB;
    ConstantBuffer lightingConstantBuffer;

    // Numerical state
    UINT shadowMapSize{};

    // Misc
    std::unique_ptr<DXD::PostProcess> postProcessForBloom;
};

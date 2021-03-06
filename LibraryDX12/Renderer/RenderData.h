#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "Descriptor/DescriptorController.h"
#include "Resource/ConstantBuffer.h"
#include "Scene/PostProcessImpl.h"
#include "Utility/AlternatingResources.h"

#include <vector>

class RenderData {
public:
    struct AlternatingResourceImpl : AlternatingResources {
        using AlternatingResources::AlternatingResources;
        void resize(int width, int height) override;
    };

    RenderData(int width, int height, UINT buffersCount);
    ~RenderData();
    void resize(int width, int height);

    // Getters
    UINT getShadowMapSize() { return shadowMapSize; }
    Resource &getShadowMap(int i) { return *shadowMap[i]; }
    Resource &getGBufferAlbedo() { return *gBufferAlbedo; }
    Resource &getGBufferNormal() { return *gBufferNormal; }
    Resource &getGBufferSpecular() { return *gBufferSpecular; }
    Resource &getSsaoMap() { return *ssaoMap; }
    Resource &getSsrMap() { return *ssrMap; }
    Resource &getSsrBlurredMap() { return *ssrBlurredMap; }
    Resource &getBloomMap() { return *bloomMap; }
    Resource &getDofMap() { return *dofMap; }
    AlternatingResources &getSceneAlternatingResources() { return sceneAlternatingResources; }
    AlternatingResources &getHelperAlternatingResources() { return helperAlternatingResources; }
    PostProcessImpl &getPostProcessForBloom() { return *static_cast<PostProcessImpl *>(postProcessForBloom.get()); }
    ConstantBuffer &getLightingConstantBuffer() { return lightingConstantBuffer; }
    VertexBuffer &getFullscreenVB() { return *fullscreenVB; }
    Resource &getDepthStencilBuffer() { return *depthStencilBuffer; };

private:
    // Buffer creations
    void createShadowMaps(unsigned int shadowsQuality);
    void createGBuffers(int width, int height);
    void createSsaoMap(int width, int height);
    void createSsrMaps(int width, int height);
    void createBloomMap(int width, int height);
    void createDofMap(int width, int height);
    void createDepthBuffer(int width, int height);

    // Helpers
    static D3D12_RESOURCE_DESC getBaseDescForFullscreenTexture(int width, int height);

    // Base objects
    ID3D12DevicePtr device = {};

    // Shadow maps
    UINT shadowMapSize{};
    std::unique_ptr<Resource> shadowMap[8] = {};

    // GBuffers
    std::unique_ptr<Resource> gBufferAlbedo;
    std::unique_ptr<Resource> gBufferNormal;
    std::unique_ptr<Resource> gBufferSpecular;

    // Effect-specific side buffers
    std::unique_ptr<Resource> ssaoMap;
    std::unique_ptr<Resource> ssrMap;
    std::unique_ptr<Resource> ssrBlurredMap;
    std::unique_ptr<Resource> bloomMap;
    std::unique_ptr<Resource> dofMap;

    // Misc
    AlternatingResourceImpl sceneAlternatingResources;
    AlternatingResourceImpl helperAlternatingResources;
    std::unique_ptr<DXD::PostProcess> postProcessForBloom;
    int lightConstantBufferIdx = 0;
    ConstantBuffer lightingConstantBuffer;
    std::unique_ptr<VertexBuffer> fullscreenVB;
    std::unique_ptr<Resource> depthStencilBuffer = {};
};

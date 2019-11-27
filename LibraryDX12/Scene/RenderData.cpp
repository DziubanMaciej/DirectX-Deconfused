#include "RenderData.h"

#include "Application/ApplicationImpl.h"
#include "ConstantBuffers/ConstantBuffers.h"
#include "Resource/VertexOrIndexBuffer.h"
#include "Utility/DxObjectNaming.h"

// ------------------------------------------------------------------------------- General

RenderData::RenderData(int width, int height)
    : device(ApplicationImpl::getInstance().getDevice()),
      postProcessRenderTargets(device),
      postProcessForBloom(DXD::PostProcess::create()),
      lightingConstantBuffer(sizeof(LightingHeapCB)) {
    // Configure bloom blur
    postProcessForBloom->setGaussianBlur(3, 5);

    // Register handler for reallocating shadow maps when shadows quality changes
    const int shadowsQuality = ApplicationImpl::getInstance().getSettingsImpl().getShadowsQuality();
    createShadowMaps(shadowsQuality);
    ApplicationImpl::getInstance().getSettingsImpl().registerHandler<SettingsImpl::ShadowsQuality>([this](const SettingsData &data) {
        this->createShadowMaps(std::get<SettingsImpl::ShadowsQuality>(data).value);
    });

    // Record command list for GPU upload and submit to gpu
    auto &commandQueue = ApplicationImpl::getInstance().getDirectCommandQueue();
    XMFLOAT3 postProcessSquare[6] =
        {
            {-1.0f, -1.0f, 0.0f},
            {-1.0f, 1.0f, 0.0f},
            {1.0f, 1.0f, 0.0f},
            {-1.0f, -1.0f, 0.0f},
            {1.0f, 1.0f, 0.0f},
            {1.0f, -1.0f, 0.0f}};
    CommandList commandList{commandQueue};
    fullscreenVB = std::make_unique<VertexBuffer>(device, commandList, postProcessSquare, 6, 12);
    SET_OBJECT_NAME(*fullscreenVB, L"FullscreenVB");
    commandList.close();
    const uint64_t fenceValue = commandQueue.executeCommandListAndSignal(commandList);
}

RenderData::~RenderData() {
    ApplicationImpl::getInstance().getSettingsImpl().unregisterHandler<SettingsImpl::ShadowsQuality>();
}

// ------------------------------------------------------------------------------- Buffer creation and resizing

void RenderData::resize(int width, int height) {
    width = std::max(static_cast<uint32_t>(width), 1u);
    height = std::max(static_cast<uint32_t>(height), 1u);

    createGBuffers(width, height);
    createSsaoMap(width, height);
    createSsrMaps(width, height);
    createBloomMap(width, height);
    createDofMap(width, height);
    createDepthBuffer(width, height);
    postProcessRenderTargets.resize(width, height);
}

void RenderData::createShadowMaps(unsigned int shadowsQuality) {
    D3D12_DEPTH_STENCIL_VIEW_DESC shadowMapDsvDesc = {};
    shadowMapDsvDesc.Format = DXGI_FORMAT_D16_UNORM;
    shadowMapDsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    shadowMapDsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    shadowMapDsvDesc.Texture2D = D3D12_TEX2D_DSV{0};
    D3D12_SHADER_RESOURCE_VIEW_DESC shadowMapSrvDesc = {};
    shadowMapSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    shadowMapSrvDesc.Format = DXGI_FORMAT_R16_UNORM;
    shadowMapSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    shadowMapSrvDesc.Texture2D.MipLevels = 1;
    shadowMapSrvDesc.Texture2D.MostDetailedMip = 0;
    shadowMapSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    shadowMapSrvDesc.Texture2D.PlaneSlice = 0;

    const UINT sizes[] = {0, 256, 512, 768, 1024, 1280, 1536, 1792, 2048, 2560, 3072};
    this->shadowMapSize = sizes[shadowsQuality];

    if (shadowMapSize == 0) {
        for (int i = 0; i < 8; i++) {
            shadowMap[i] = std::make_unique<Resource>();
            shadowMap[i]->createNullSrv(&shadowMapSrvDesc);
        }
        return;
    }

    for (int i = 0; i < 8; i++) {
        shadowMap[i] = std::make_unique<Resource>(device,
                                                  &CD3DX12_HEAP_PROPERTIES{D3D12_HEAP_TYPE_DEFAULT},
                                                  D3D12_HEAP_FLAG_NONE,
                                                  &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R16_TYPELESS, shadowMapSize, shadowMapSize, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
                                                  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                                  &CD3DX12_CLEAR_VALUE{DXGI_FORMAT_D16_UNORM, 1.0f, 0});
        shadowMap[i]->createDsv(&shadowMapDsvDesc);
        shadowMap[i]->createSrv(&shadowMapSrvDesc);
        SET_OBJECT_NAME(*shadowMap[i], L"ShadowMap%d", i);
    }
}

void RenderData::createGBuffers(int width, int height) {
    // GBuffer Albedo
    D3D12_RESOURCE_DESC gBufferAlbedoDesc = getBaseDescForFullscreenTexture(width, height);
    gBufferAlbedoDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    gBufferAlbedoDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    gBufferAlbedo = std::make_unique<Resource>(
        device,
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &gBufferAlbedoDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        nullptr);
    gBufferAlbedo->createSrv(nullptr);
    gBufferAlbedo->createRtv(nullptr);
    SET_OBJECT_NAME(*gBufferAlbedo, L"GBuffer Albedo");

    // GBuffer Normal
    D3D12_RESOURCE_DESC gBufferNormalDesc = getBaseDescForFullscreenTexture(width, height);
    gBufferNormalDesc.Format = DXGI_FORMAT_R16G16B16A16_SNORM;
    gBufferNormalDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    gBufferNormal = std::make_unique<Resource>(
        device,
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &gBufferNormalDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        nullptr);
    gBufferNormal->createSrv(nullptr);
    gBufferNormal->createRtv(nullptr);
    SET_OBJECT_NAME(*gBufferNormal, L"GBuffer Normal");

    // GBuffer Specular
    D3D12_RESOURCE_DESC gBufferSpecularDesc = getBaseDescForFullscreenTexture(width, height);
    gBufferSpecularDesc.Format = DXGI_FORMAT_R8G8_UNORM;
    gBufferSpecularDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    gBufferSpecular = std::make_unique<Resource>(
        device,
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &gBufferSpecularDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        nullptr);
    gBufferSpecular->createSrv(nullptr);
    gBufferSpecular->createRtv(nullptr);
    SET_OBJECT_NAME(*gBufferSpecular, L"GBuffer Specular");
}

void RenderData::createSsaoMap(int width, int height) {
    width = std::max(width / 2, 1);
    height = std::max(height / 2, 1);
    D3D12_RESOURCE_DESC ssaoMapDesc = getBaseDescForFullscreenTexture(width, height);
    ssaoMapDesc.Format = DXGI_FORMAT_R8_UNORM;
    ssaoMapDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    ssaoMap = std::make_unique<Resource>(
        device,
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &ssaoMapDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr);
    ssaoMap->createSrv(nullptr);
    ssaoMap->createRtv(nullptr);
    SET_OBJECT_NAME(*ssaoMap, L"SsaoMap");
}

void RenderData::createSsrMaps(int width, int height) {
    // SSR map
    int halfWidth = std::max(width / 2, 1);
    int halfHeight = std::max(height / 2, 1);
    D3D12_RESOURCE_DESC ssrMapDesc = getBaseDescForFullscreenTexture(halfWidth, halfHeight);
    ssrMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    ssrMapDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    ssrMap = std::make_unique<Resource>(
        device,
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &ssrMapDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr);
    ssrMap->createSrv(nullptr);
    ssrMap->createRtv(nullptr);
    SET_OBJECT_NAME(*ssrMap, L"SsrMap");

    // SSR blurred map
    D3D12_RESOURCE_DESC ssrBlurredMapDesc = getBaseDescForFullscreenTexture(width, height);
    ssrBlurredMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    ssrBlurredMapDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    ssrBlurredMap = std::make_unique<Resource>(
        device,
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &ssrBlurredMapDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr);
    ssrBlurredMap->createSrv(nullptr);
    ssrBlurredMap->createRtv(nullptr);
    SET_OBJECT_NAME(*ssrBlurredMap, L"SsrBlurredMap");
}

void RenderData::createBloomMap(int width, int height) {
    D3D12_RESOURCE_DESC bloomMapDesc = getBaseDescForFullscreenTexture(width, height);
    bloomMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bloomMapDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    bloomMap = std::make_unique<Resource>(
        device,
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &bloomMapDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        nullptr);
    bloomMap->createSrv(nullptr);
    bloomMap->createRtv(nullptr);
    bloomMap->createUav(nullptr);
    SET_OBJECT_NAME(*bloomMap, L"BloomMap");
}

void RenderData::createDofMap(int width, int height) {
    D3D12_RESOURCE_DESC dofMapDesc = getBaseDescForFullscreenTexture(width, height);
    dofMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    dofMapDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    dofMap = std::make_unique<Resource>(
        device,
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &dofMapDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr);
    dofMap->createSrv(nullptr);
    dofMap->createRtv(nullptr);
    SET_OBJECT_NAME(*dofMap, L"dofMap");
}

void RenderData::createDepthBuffer(int width, int height) {
    depthStencilBuffer = std::make_unique<Resource>(device,
                                                    &CD3DX12_HEAP_PROPERTIES{D3D12_HEAP_TYPE_DEFAULT},
                                                    D3D12_HEAP_FLAG_NONE,
                                                    &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
                                                    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                                    &CD3DX12_CLEAR_VALUE{DXGI_FORMAT_D32_FLOAT, 1.0f, 0});
    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilBufferDesc = {};
    depthStencilBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilBufferDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilBufferDesc.Flags = D3D12_DSV_FLAG_NONE;
    depthStencilBufferDesc.Texture2D = D3D12_TEX2D_DSV{0};
    D3D12_SHADER_RESOURCE_VIEW_DESC depthStencilBufferSrvDesc = {};
    depthStencilBufferSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    depthStencilBufferSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    depthStencilBufferSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    depthStencilBufferSrvDesc.Texture2D.MipLevels = 1;
    depthStencilBufferSrvDesc.Texture2D.MostDetailedMip = 0;
    depthStencilBufferSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    depthStencilBufferSrvDesc.Texture2D.PlaneSlice = 0;
    depthStencilBuffer->createDsv(&depthStencilBufferDesc);
    depthStencilBuffer->createSrv(&depthStencilBufferSrvDesc);
    SET_OBJECT_NAME(*depthStencilBuffer, L"DepthStencilBuffer");
}

void RenderData::PostProcessRenderTargets::resize(int width, int height) {
    D3D12_RESOURCE_DESC renderTargetDesc = getBaseDescForFullscreenTexture(width, height);
    renderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    for (int i = 0; i <= 1; i++) { // Initialize resource0 and resource1 in the same way
        resources[i] = std::make_unique<Resource>(
            device,
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &renderTargetDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr);
        resources[i]->createSrv(nullptr);
        resources[i]->createRtv(nullptr);
        resources[i]->createUav(nullptr);
        SET_OBJECT_NAME(*resources[i], L"PostProcessRT%d", i);
    }
}

// ------------------------------------------------------------------------------- Helpers
inline D3D12_RESOURCE_DESC RenderData::getBaseDescForFullscreenTexture(int width, int height) {
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    return desc;
}

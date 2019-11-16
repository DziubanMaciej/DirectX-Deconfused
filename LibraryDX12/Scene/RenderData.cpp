#include "RenderData.h"

#include "Application/ApplicationImpl.h"
#include "ConstantBuffers/ConstantBuffers.h"
#include "Utility/DxObjectNaming.h"

RenderData::RenderData(ID3D12DevicePtr &device, DescriptorController &descriptorController, int width, int height)
    : device(device),
      postProcessRenderTargets(device),
      postProcessForBloom(DXD::PostProcess::create()) {
    postProcessForBloom->setGaussianBlur(3, 5);
    ApplicationImpl::getInstance().getSettingsImpl().registerHandler<SettingsImpl::ShadowsQuality>([this](const SettingsData &data) {
        this->createShadowMaps(std::get<SettingsImpl::ShadowsQuality>(data).value);
    });
}

RenderData::~RenderData() {
    ApplicationImpl::getInstance().getSettingsImpl().unregisterHandler<SettingsImpl::ShadowsQuality>();
}

void RenderData::resize(int width, int height) {
    width = std::max(static_cast<uint32_t>(width), 1u);
    height = std::max(static_cast<uint32_t>(height), 1u);
    postProcessRenderTargets.resize(width, height);

    // GBuffer Albedo
    D3D12_RESOURCE_DESC gBufferAlbedoDesc = {};
    gBufferAlbedoDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    gBufferAlbedoDesc.Alignment = 0;
    gBufferAlbedoDesc.Width = width;
    gBufferAlbedoDesc.Height = height;
    gBufferAlbedoDesc.DepthOrArraySize = 1;
    gBufferAlbedoDesc.MipLevels = 0;
    gBufferAlbedoDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    gBufferAlbedoDesc.SampleDesc.Count = 1;
    gBufferAlbedoDesc.SampleDesc.Quality = 0;
    gBufferAlbedoDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
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
    gBufferAlbedo->getResource()->SetName(L"GBuffer Albedo");

    // GBuffer Normal
    D3D12_RESOURCE_DESC gBufferNormalDesc = {};
    gBufferNormalDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    gBufferNormalDesc.Alignment = 0;
    gBufferNormalDesc.Width = width;
    gBufferNormalDesc.Height = height;
    gBufferNormalDesc.DepthOrArraySize = 1;
    gBufferNormalDesc.MipLevels = 0;
    gBufferNormalDesc.Format = DXGI_FORMAT_R16G16B16A16_SNORM;
    gBufferNormalDesc.SampleDesc.Count = 1;
    gBufferNormalDesc.SampleDesc.Quality = 0;
    gBufferNormalDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
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
    gBufferNormal->getResource()->SetName(L"GBuffer Normal");

    //GBuffer Specular
    D3D12_RESOURCE_DESC gBufferSpecularDesc = {};
    gBufferSpecularDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    gBufferSpecularDesc.Alignment = 0;
    gBufferSpecularDesc.Width = width;
    gBufferSpecularDesc.Height = height;
    gBufferSpecularDesc.DepthOrArraySize = 1;
    gBufferSpecularDesc.MipLevels = 0;
    gBufferSpecularDesc.Format = DXGI_FORMAT_R8G8_UNORM;
    gBufferSpecularDesc.SampleDesc.Count = 1;
    gBufferSpecularDesc.SampleDesc.Quality = 0;
    gBufferSpecularDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
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
    gBufferSpecular->getResource()->SetName(L"GBuffer Specular");

    // Bloom map
    D3D12_RESOURCE_DESC renderTargetDesc = {};
    renderTargetDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    renderTargetDesc.Alignment = 0;
    renderTargetDesc.Width = width;
    renderTargetDesc.Height = height;
    renderTargetDesc.DepthOrArraySize = 1;
    renderTargetDesc.MipLevels = 0;
    renderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    renderTargetDesc.SampleDesc.Count = 1;
    renderTargetDesc.SampleDesc.Quality = 0;
    renderTargetDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    bloomMap = std::make_unique<Resource>(
        device,
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &renderTargetDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        nullptr);
    bloomMap->createSrv(nullptr);
    bloomMap->createRtv(nullptr);
    bloomMap->createUav(nullptr);
    bloomMap->getResource()->SetName(L"Bloom map");
    SET_OBJECT_NAME(*bloomMap, L"BloomMap");

    // SSAO map
    D3D12_RESOURCE_DESC ssaoMapDesc = {};
    ssaoMapDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    ssaoMapDesc.Alignment = 0;
    ssaoMapDesc.Width = std::max(width / 2, 1);
    ssaoMapDesc.Height = std::max(height / 2, 1);
    ssaoMapDesc.DepthOrArraySize = 1;
    ssaoMapDesc.MipLevels = 0;
    ssaoMapDesc.Format = DXGI_FORMAT_R8_UNORM;
    ssaoMapDesc.SampleDesc.Count = 1;
    ssaoMapDesc.SampleDesc.Quality = 0;
    ssaoMapDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
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
    ssaoMap->getResource()->SetName(L"SSAO map");
    SET_OBJECT_NAME(*ssaoMap, L"SsaoMap");

    // SSR map
    D3D12_RESOURCE_DESC ssrMapDesc = {};
    ssrMapDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    ssrMapDesc.Alignment = 0;
    ssrMapDesc.Width = std::max(width / 2, 1);
    ssrMapDesc.Height = std::max(height / 2, 1);
    ssrMapDesc.DepthOrArraySize = 1;
    ssrMapDesc.MipLevels = 0;
    ssrMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    ssrMapDesc.SampleDesc.Count = 1;
    ssrMapDesc.SampleDesc.Quality = 0;
    ssrMapDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
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
    ssrMap->getResource()->SetName(L"SSR map");
    SET_OBJECT_NAME(*ssrMap, L"SsrMap");

    // SSR blurred map
    D3D12_RESOURCE_DESC ssrBlurredMapDesc = {};
    ssrBlurredMapDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    ssrBlurredMapDesc.Alignment = 0;
    ssrBlurredMapDesc.Width = width;
    ssrBlurredMapDesc.Height = height;
    ssrBlurredMapDesc.DepthOrArraySize = 1;
    ssrBlurredMapDesc.MipLevels = 0;
    ssrBlurredMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    ssrBlurredMapDesc.SampleDesc.Count = 1;
    ssrBlurredMapDesc.SampleDesc.Quality = 0;
    ssrBlurredMapDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
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
    ssrBlurredMap->getResource()->SetName(L"SSR blurred map");
    SET_OBJECT_NAME(*ssrBlurredMap, L"SsrBlurredMap");

    // Lighting output
    D3D12_RESOURCE_DESC loDesc = {};
    loDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    loDesc.Alignment = 0;
    loDesc.Width = width;
    loDesc.Height = height;
    loDesc.DepthOrArraySize = 1;
    loDesc.MipLevels = 0;
    loDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    loDesc.SampleDesc.Count = 1;
    loDesc.SampleDesc.Quality = 0;
    loDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    loDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    lightingOutput = std::make_unique<Resource>(
        device,
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &loDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr);
    lightingOutput->createSrv(nullptr);
    lightingOutput->createRtv(nullptr);
    lightingOutput->getResource()->SetName(L"LO");
    SET_OBJECT_NAME(*lightingOutput, L"LO");

    const int shadowsQuality = ApplicationImpl::getInstance().getSettingsImpl().getShadowsQuality();
    createShadowMaps(shadowsQuality);

    // Depth stencil buffer
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

void PostProcessRenderTargets::resize(int width, int height) {
    //Description
    D3D12_RESOURCE_DESC renderTargetDesc;
    renderTargetDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    renderTargetDesc.Alignment = 0;
    renderTargetDesc.Width = width;
    renderTargetDesc.Height = height;
    renderTargetDesc.DepthOrArraySize = 1;
    renderTargetDesc.MipLevels = 0;
    renderTargetDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    renderTargetDesc.SampleDesc.Count = 1;
    renderTargetDesc.SampleDesc.Quality = 0;
    renderTargetDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    for (int i = 0; i <= 1; i++) { // Initialize resource0 and resource1 the same way
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

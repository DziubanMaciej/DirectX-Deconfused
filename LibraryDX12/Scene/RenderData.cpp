#include "RenderData.h"

#include "Resource/ConstantBuffers.h"

RenderData::RenderData(ID3D12DevicePtr &device, DescriptorController &descriptorController, int width, int height)
    : device(device),
      postProcessRenderTargets(device),
      postProcessForBloom(DXD::PostProcess::create()) {
    postProcessForBloom->setGaussianBlur(3, 5);
    postProcessesForBloom.push_back(static_cast<PostProcessImpl *>(postProcessForBloom.get()));
}

void RenderData::resize(int width, int height) {
    width = std::max(static_cast<uint32_t>(width), 1u);
    height = std::max(static_cast<uint32_t>(height), 1u);
    postProcessRenderTargets.resize(width, height);

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
    renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    bloomMap = std::make_unique<Resource>(
        device,
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &renderTargetDesc,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        nullptr);
    bloomMap->createSrv(nullptr);
    bloomMap->createRtv(nullptr);
    bloomMap->getResource()->SetName(L"Bloom map");

    // Shadow maps
    D3D12_DEPTH_STENCIL_VIEW_DESC shadowMapDsvDesc = {};
    shadowMapDsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    shadowMapDsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    shadowMapDsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    shadowMapDsvDesc.Texture2D = D3D12_TEX2D_DSV{0};
    D3D12_SHADER_RESOURCE_VIEW_DESC shadowMapSrvDesc = {};
    shadowMapSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    shadowMapSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    shadowMapSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    shadowMapSrvDesc.Texture2D.MipLevels = 1;
    shadowMapSrvDesc.Texture2D.MostDetailedMip = 0;
    shadowMapSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    shadowMapSrvDesc.Texture2D.PlaneSlice = 0;
    for (int i = 0; i < 8; i++) {
        // Resource
        shadowMap[i] = std::make_unique<Resource>(device,
                                                  &CD3DX12_HEAP_PROPERTIES{D3D12_HEAP_TYPE_DEFAULT},
                                                  D3D12_HEAP_FLAG_NONE, // TODO D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES good?
                                                  &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, static_cast<uint32_t>(2048), static_cast<uint32_t>(2048), 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
                                                  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                                  &CD3DX12_CLEAR_VALUE{DXGI_FORMAT_D32_FLOAT, 1.0f, 0});
        shadowMap[i]->createDsv(&shadowMapDsvDesc);
        shadowMap[i]->createSrv(&shadowMapSrvDesc);
    }

    // Depth stencil buffer
    depthStencilBuffer = std::make_unique<Resource>(device,
                                                    &CD3DX12_HEAP_PROPERTIES{D3D12_HEAP_TYPE_DEFAULT},
                                                    D3D12_HEAP_FLAG_NONE, // TODO D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES good?
                                                    &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
                                                    D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                    &CD3DX12_CLEAR_VALUE{DXGI_FORMAT_D32_FLOAT, 1.0f, 0});
    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilBufferDesc = {};
    depthStencilBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilBufferDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilBufferDesc.Flags = D3D12_DSV_FLAG_NONE;
    depthStencilBufferDesc.Texture2D = D3D12_TEX2D_DSV{0};
    depthStencilBuffer->createDsv(&depthStencilBufferDesc);
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
    renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

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
    }
}

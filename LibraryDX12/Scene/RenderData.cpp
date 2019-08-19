#include "RenderData.h"

#include "Resource/ConstantBuffers.h"

RenderData::RenderData(ID3D12DevicePtr &device, DescriptorController &descriptorController, int width, int height)
    : device(device),
      postProcessSrvDescriptor(descriptorController.allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1)),
      postProcessRtvDescriptor(descriptorController.allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1)),
      postProcessConvolutionCb(device, descriptorController, sizeof(PostProcessConvolutionCB)),
      shadowMapDsvDescriptors(descriptorController.allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 8)),
      shadowMapSrvDescriptors(descriptorController.allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 8)),
      dsvDescriptor(descriptorController.allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1)) {
}

void RenderData::resize(int width, int height) {
    width = std::max(width, 1);
    height = std::max(height, 1);

    auto data = postProcessConvolutionCb.getData<PostProcessConvolutionCB>();
    data->screenWidth = width;
    data->screenHeight = height;

    data->kernel = XMFLOAT4X3(1.f, 2.f, 1.f, 0.f, 2.f, 4.f, 2.f, 0.f, 1.f, 2.f, 1.f, 0.f);
    data->sum = 16;

    data->kernel = XMFLOAT4X3(-1.f, -1.f, -1.f, 0.f, -1.f, 8.f, -1.f, 0.f, -1.f, -1.f, -1.f, 0.f);
    data->sum = 1;

    //data->kernel = XMFLOAT3X3(-1.f, -1.f, -1.f, -1.f, 8.f, -1.f, -1.f, -1.f, -1.f);
    //data->sum = 1.f;

    //data->kernel = XMFLOAT3X3(1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f);
    //data->sum = 9.f;

    //data->kernel = XMFLOAT4X3(-1.f, -1.f, -1.f, -1.f, 8.f, -1.f, -1.f, -1.f, -1.f, 0.f, 0.f, 0.f);
    // data->sum = 1;

    // data->kernel = XMFLOAT4X3(0.1f, 0.2f, 0.3f, 0.f, 0.4f, 0.5f, 0.6f, 0.f, 0.7f, 0.8f, 0.9f, 0.f);
    // data->sum = 1;
    postProcessConvolutionCb.upload();

    //Post process render target
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
    postProcessRenderTarget = std::make_unique<Resource>(
        device,
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &renderTargetDesc,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
        nullptr);
    device->CreateRenderTargetView(postProcessRenderTarget->getResource().Get(), nullptr, postProcessRtvDescriptor.getCpuHandle());
    device->CreateShaderResourceView(postProcessRenderTarget->getResource().Get(), nullptr, postProcessSrvDescriptor.getCpuHandle());

    // Shadow maps
    for (int i = 0; i < 8; i++) {
        // Resource
        shadowMap[i] = std::make_unique<Resource>(device,
                                                  &CD3DX12_HEAP_PROPERTIES{D3D12_HEAP_TYPE_DEFAULT},
                                                  D3D12_HEAP_FLAG_NONE, // TODO D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES good?
                                                  &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, static_cast<uint32_t>(2048), static_cast<uint32_t>(2048), 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
                                                  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                                  &CD3DX12_CLEAR_VALUE{DXGI_FORMAT_D32_FLOAT, 1.0f, 0});

        // DSV
        D3D12_DEPTH_STENCIL_VIEW_DESC shadowMapDesc = {};
        shadowMapDesc.Format = DXGI_FORMAT_D32_FLOAT;
        shadowMapDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        shadowMapDesc.Flags = D3D12_DSV_FLAG_NONE;
        shadowMapDesc.Texture2D = D3D12_TEX2D_DSV{0};
        device->CreateDepthStencilView(shadowMap[i]->getResource().Get(), &shadowMapDesc, shadowMapDsvDescriptors.getCpuHandle(i));

        // SRV
        D3D12_SHADER_RESOURCE_VIEW_DESC shadowMapSrvDesc = {};
        shadowMapSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        shadowMapSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        shadowMapSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        shadowMapSrvDesc.Texture2D.MipLevels = 1;
        shadowMapSrvDesc.Texture2D.MostDetailedMip = 0;
        shadowMapSrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        shadowMapSrvDesc.Texture2D.PlaneSlice = 0;
        device->CreateShaderResourceView(shadowMap[i]->getResource().Get(), &shadowMapSrvDesc, shadowMapSrvDescriptors.getCpuHandle(i));
    }

    depthStencilBuffer = std::make_unique<Resource>(device,
                                                    &CD3DX12_HEAP_PROPERTIES{D3D12_HEAP_TYPE_DEFAULT},
                                                    D3D12_HEAP_FLAG_NONE, // TODO D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES good?
                                                    &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
                                                    D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                    &CD3DX12_CLEAR_VALUE{DXGI_FORMAT_D32_FLOAT, 1.0f, 0});

    // Depth stencil buffer
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.Texture2D = D3D12_TEX2D_DSV{0};
    device->CreateDepthStencilView(
        depthStencilBuffer->getResource().Get(),
        &dsvDesc,
        dsvDescriptor.getCpuHandle());
}

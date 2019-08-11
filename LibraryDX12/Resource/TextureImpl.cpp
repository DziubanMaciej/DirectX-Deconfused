#include "TextureImpl.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"
#include "Utility/FileHelper.h"
#include "Utility/ThrowIfFailed.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include "DXD/ExternalHeadersWrappers/stb_image.h"
#include <cassert>
#include <cstdlib>

namespace DXD {
std::unique_ptr<Texture> Texture::createFromFile(Application &application, const std::wstring &filePath) {
    // TODO if texture used as Albedo (Diffuse), make SRGB, e.g. convert DXGI_FORMAT_R8G8B8A8_UNORM to DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
    // TODO create views
    // TODO generate mips

    const auto fullFilePath = std::wstring{RESOURCES_PATH} + filePath;
    if (!FileHelper::exists(fullFilePath)) {
        return nullptr;
    }

    DirectX::TexMetadata metadata;
    DirectX::ScratchImage scratchImage;

    /*throwIfFailed(DirectX::LoadFromTGAFile(
        filePathW.get(),
        &metadata,
        scratchImage))*/
    ;

    throwIfFailed(DirectX::LoadFromWICFile(
        fullFilePath.c_str(),
        DirectX::WIC_FLAGS_FORCE_RGB,
        &metadata,
        scratchImage));

    const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UINT;
    const D3D12_RESOURCE_DESC description = TextureImpl::createTextureDescription(metadata);
    return std::unique_ptr<Texture>(new TextureImpl(*static_cast<ApplicationImpl *>(&application), description, filePath, scratchImage));
}
} // namespace DXD

TextureImpl::TextureImpl(ApplicationImpl &application, const D3D12_RESOURCE_DESC &description, const std::wstring &fileName, const DirectX::ScratchImage &image)
    : Resource(application.getDevice(), &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &description, D3D12_RESOURCE_STATE_COPY_DEST, nullptr),
      fileName(fileName), description(description),
      cpuDescriptors(application.getDescriptorController().allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1)) {

    // Create command list
    CommandQueue &commandQueue = application.getDirectCommandQueue();
    CommandList commandList{commandQueue.getCommandAllocatorManager(), nullptr};

    // Create buffer on upload heap
    Resource intermediateResource(application.getDevice(), D3D12_HEAP_TYPE_UPLOAD, D3D12_HEAP_FLAG_NONE, GetRequiredIntermediateSize(resource.Get(), 0, 1), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);

    // Transfer data through the upload heap to destination resource
    const auto resourceDescription = resource->GetDesc();
    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = image.GetPixels();
    subresourceData.RowPitch = image.GetImages()->rowPitch;
    subresourceData.SlicePitch = image.GetImages()->slicePitch;
    UpdateSubresources<1>(commandList.getCommandList().Get(), this->resource.Get(), intermediateResource.getResource().Get(), 0, 0, 1, &subresourceData);

    // Transition destination resource and make intermediateResource tracked so it's not deleted while being processed on the GPU
    transitionBarrierSingle(commandList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.addUsedResource(intermediateResource.getResource());

    // Send command list
    commandList.close();
    std::vector<CommandList *> commandLists{&commandList};
    commandQueue.executeCommandListsAndSignal(commandLists);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDescription = {};
    srvDescription.Format = this->description.Format;
    srvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDescription.Texture2D.MipLevels = 1;
    srvDescription.Texture2D.MostDetailedMip = 0;
    srvDescription.Texture2D.PlaneSlice = 0;
    srvDescription.Texture2D.ResourceMinLODClamp = 0;
    application.getDevice()->CreateShaderResourceView(this->resource.Get(), &srvDescription, cpuDescriptors.getCpuHandle(0));
}

D3D12_RESOURCE_DIMENSION TextureImpl::calculateTextureDimension(int width, int height) {
    if (height == 1) {
        return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    }
    return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
}

D3D12_RESOURCE_DESC TextureImpl::createTextureDescription(const DirectX::TexMetadata &metadata) {
    D3D12_RESOURCE_DESC textureDesc{};
    switch (metadata.dimension) {
    case DirectX::TEX_DIMENSION_TEXTURE1D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(
            metadata.format,
            static_cast<UINT64>(metadata.width),
            static_cast<UINT16>(metadata.arraySize));
        break;
    case DirectX::TEX_DIMENSION_TEXTURE2D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            metadata.format,
            static_cast<UINT64>(metadata.width),
            static_cast<UINT>(metadata.height),
            static_cast<UINT16>(metadata.arraySize));
        break;
    case DirectX::TEX_DIMENSION_TEXTURE3D:
        textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(
            metadata.format,
            static_cast<UINT64>(metadata.width),
            static_cast<UINT>(metadata.height),
            static_cast<UINT16>(metadata.depth));
        break;
    default:
        UNREACHABLE_CODE();
    }
    return textureDesc;
}

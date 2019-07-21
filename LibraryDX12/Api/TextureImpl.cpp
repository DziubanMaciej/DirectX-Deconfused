#include "TextureImpl.h"

#include "Api/ApplicationImpl.h"
#include "Utility/FileHelper.h"
#include "Utility/ThrowIfFailed.h"
#include "Wrappers/CommandList.h"
#include "Wrappers/CommandQueue.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include "DXD/ExternalHeadersWrappers/stb_image.h"
#include <cassert>

namespace DXD {
std::unique_ptr<Texture> Texture::createFromFile(Application &application, const std::string &filePath) {
    // TODO is texture used as Albedo (Diffuse), make SRGB, e.g. convert DXGI_FORMAT_R8G8B8A8_UNORM to DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
    // TODO array_size
    // TODO 3D textures
    // TODO hardcoded format
    // TODO create views
    // TODO generate mips

    if (!FileHelper::exists(filePath)) {
        return nullptr;
    }

    const StbImage image(filePath, STBI_rgb);
    const D3D12_RESOURCE_DIMENSION dimension = TextureImpl::calculateTextureDimension(image.width, image.height);
    const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UINT;
    const D3D12_RESOURCE_DESC description = TextureImpl::createTextureDescription(dimension, format, image.width, image.height);
    return std::unique_ptr<Texture>(new TextureImpl(*static_cast<ApplicationImpl *>(&application), description, filePath, image.data));
}
} // namespace DXD

TextureImpl::TextureImpl(ApplicationImpl &application, const D3D12_RESOURCE_DESC &description, const std::string &fileName, unsigned char *imageData)
    : Resource(application.getDevice(), &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &description, D3D12_RESOURCE_STATE_COPY_DEST, nullptr),
      fileName(fileName), description(description) {
    upload(application, imageData, description);
}

D3D12_RESOURCE_DIMENSION TextureImpl::calculateTextureDimension(int width, int height) {
    if (height == 1) {
        return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
    }
    return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
}

D3D12_RESOURCE_DESC TextureImpl::createTextureDescription(D3D12_RESOURCE_DIMENSION dimension, DXGI_FORMAT format, int width, int height) {
    const auto w = static_cast<UINT64>(width);
    const auto h = static_cast<UINT>(height);

    CD3DX12_RESOURCE_DESC description;
    switch (dimension) {
    case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        description = CD3DX12_RESOURCE_DESC::Tex1D(format, w);
        break;
    case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        description = CD3DX12_RESOURCE_DESC::Tex2D(format, w, h);
        break;
    default:
        assert(false);
    }
    return description;
}

void TextureImpl::upload(ApplicationImpl &application, unsigned char *imageData, const D3D12_RESOURCE_DESC &textureDescription) {
    CommandQueue &commandQueue = application.getDirectCommandQueue();
    CommandList commandList{commandQueue.getCommandAllocatorManager(), nullptr};

    ID3D12ResourcePtr intermediateResource{};
    throwIfFailed(application.getDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(GetRequiredIntermediateSize(resource.Get(), 0, 1)),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&intermediateResource)));

    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = imageData;
    vertexData.RowPitch = textureDescription.Width; // TODO: row pitch is in bytes, width is in texels
    vertexData.SlicePitch = textureDescription.Height;

    UpdateSubresources<1>(commandList.getCommandList().Get(), resource.Get(), intermediateResource.Get(), 0, 0, 1, &vertexData);
    commandList.transitionBarrierSingle(resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE| D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    commandList.addUsedResource(intermediateResource);

    std::vector<CommandList *> commandLists{&commandList};
    commandList.close();
    commandQueue.executeCommandListsAndSignal(commandLists);
}
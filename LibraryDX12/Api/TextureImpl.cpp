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
    uploadToGPU(application, imageData, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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

#include "TextureImpl.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"
#include "Utility/FileHelper.h"
#include "Utility/ThrowIfFailed.h"

#include <ExternalHeaders/Wrappers/d3dx12.h>
#include <cassert>
#include <cstdlib>

// ----------------------------------------------------------------- Creation and destruction

namespace DXD {
std::unique_ptr<Texture> Texture::createFromFile(Application &application, const std::wstring &filePath, bool asynchronousLoading) {
    return std::unique_ptr<Texture>(new TextureImpl(*static_cast<ApplicationImpl *>(&application), filePath, asynchronousLoading));
}
} // namespace DXD

TextureImpl::TextureImpl(ApplicationImpl &application, const std::wstring &filePath, bool asynchronousLoading) {
    const TextureCpuLoadArgs cpuLoadArgs{filePath};
    this->cpuGpuLoad(cpuLoadArgs, asynchronousLoading);
}

TextureImpl::~TextureImpl() {
    terminateBackgroundProcessing(true);
}

// ----------------------------------------------------------------- AsyncLoadableObject overrides

TextureCpuLoadResult TextureImpl::cpuLoad(const TextureCpuLoadArgs &args) {
    // Validate path
    const auto fullFilePath = std::wstring{RESOURCES_PATH} + args.filePath;
    if (!FileHelper::exists(fullFilePath)) {
        return std::move(TextureCpuLoadResult{});
    }

    // Load image
    TextureCpuLoadResult result = {};
    const auto extension = FileHelper::getExtension(args.filePath, true);
    if (extension == L"tga") {
        throwIfFailed(DirectX::LoadFromTGAFile(
            fullFilePath.c_str(),
            &result.metadata,
            result.scratchImage));
    } else if (extension == L"dds") {
        throwIfFailed(DirectX::LoadFromDDSFile(
            fullFilePath.c_str(),
            DirectX::DDS_FLAGS_FORCE_RGB,
            &result.metadata,
            result.scratchImage));
    } else if (extension == L"hdr") {
        throwIfFailed(DirectX::LoadFromHDRFile(
            fullFilePath.c_str(),
            &result.metadata,
            result.scratchImage));
    } else if (extension == L"jpg" || extension == L"png" || extension == L"bmp") {
        throwIfFailed(DirectX::LoadFromWICFile(
            fullFilePath.c_str(),
            DirectX::WIC_FLAGS_FORCE_RGB,
            &result.metadata,
            result.scratchImage));
    } else {
        UNREACHABLE_CODE();
    }

    // Return successfully
    result.success = true;
    return std::move(result);
}

bool TextureImpl::isCpuLoadSuccessful(const TextureCpuLoadResult &result) {
    return result.success;
}

TextureGpuLoadArgs TextureImpl::createArgsForGpuLoad(const TextureCpuLoadResult &cpuLoadResult) {
    return std::move(TextureGpuLoadArgs{
        cpuLoadResult.metadata,
        cpuLoadResult.scratchImage});
}

TextureGpuLoadResult TextureImpl::gpuLoad(const TextureGpuLoadArgs &args) {
    TextureGpuLoadResult result = {};

    // Create GPU resource
    result.description = TextureImpl::createTextureDescription(args.metadata);
    this->setResource(createResource(ApplicationImpl::getInstance().getDevice(), &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                                     &result.description, D3D12_RESOURCE_STATE_COPY_DEST, nullptr),
                      D3D12_RESOURCE_STATE_COPY_DEST);

    // Upload data to the GPU resource
    Resource::uploadToGPU(ApplicationImpl::getInstance(), args.scratchImage.GetPixels(),
                          static_cast<UINT>(args.scratchImage.GetImages()->rowPitch),
                          static_cast<UINT>(args.scratchImage.GetImages()->slicePitch));

    // Create SRV
    DescriptorAllocation cpuDescriptors = ApplicationImpl::getInstance().getDescriptorController().allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDescription = {};
    srvDescription.Format = result.description.Format;
    srvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDescription.Texture2D.MipLevels = 1;
    srvDescription.Texture2D.MostDetailedMip = 0;
    srvDescription.Texture2D.PlaneSlice = 0;
    srvDescription.Texture2D.ResourceMinLODClamp = 0;
    this->createSrv(&srvDescription);

    return std::move(result);
}

bool TextureImpl::hasGpuLoadEnded() {
    return !Resource::isUploadInProgress();
}

void TextureImpl::writeCpuGpuLoadResults(TextureCpuLoadResult &cpuLoadResult, TextureGpuLoadResult &gpuLoadResult) {
    this->description = gpuLoadResult.description;
}

// ----------------------------------------------------------------- Helpers

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

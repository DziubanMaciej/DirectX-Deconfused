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
    return std::unique_ptr<Texture>(new TextureImpl(*static_cast<ApplicationImpl *>(&application), filePath));
}
} // namespace DXD

TextureImpl::TextureImpl(ApplicationImpl &application, const std::wstring &filePath) {
    auto task = [this, &application, filePath]() {
        loadAndUpload(application, filePath);
    };
    application.getBackgroundWorkerController().pushTask(task, this->loadingComplete);
}

void TextureImpl::loadAndUpload(ApplicationImpl &application, const std::wstring &filePath) {
    // Load on CPU
    const LoadResults loadResults = loadOnCpu(application, filePath);
    assert(loadResults.success);
    const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UINT;
    D3D12_RESOURCE_DESC description = TextureImpl::createTextureDescription(loadResults.metadata);
    const auto &image = loadResults.scratchImage;
    const auto &metadata = loadResults.metadata;

    // Create GPU resource
    this->create(application.getDevice(), &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                 &description, D3D12_RESOURCE_STATE_COPY_DEST, nullptr);

    // Upload data to the GPU resource
    uploadToGPU(application, image.GetPixels(), static_cast<UINT>(image.GetImages()->rowPitch), static_cast<UINT>(image.GetImages()->slicePitch));

    // Create SRV
    DescriptorAllocation cpuDescriptors = application.getDescriptorController().allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDescription = {};
    srvDescription.Format = this->description.Format;
    srvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDescription.Texture2D.MipLevels = 1;
    srvDescription.Texture2D.MostDetailedMip = 0;
    srvDescription.Texture2D.PlaneSlice = 0;
    srvDescription.Texture2D.ResourceMinLODClamp = 0;
    application.getDevice()->CreateShaderResourceView(this->resource.Get(), &srvDescription, cpuDescriptors.getCpuHandle(0));

    // Set all the data to TextureImpl object
    setData(std::move(cpuDescriptors), std::move(description), filePath);
}

TextureImpl::LoadResults TextureImpl::loadOnCpu(ApplicationImpl &application, const std::wstring &filePath) {
    LoadResults result = {};

    const auto fullFilePath = std::wstring{RESOURCES_PATH} + filePath;
    if (!FileHelper::exists(fullFilePath)) {
        return std::move(result);
    }

    const auto extension = FileHelper::getExtension(filePath, true);
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

    result.success = true;
    return result;
}

bool TextureImpl::isUploadInProgress() {
    return loadingComplete.load() && Resource::isUploadInProgress();
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

void TextureImpl::setData(DescriptorAllocation &&cpuDescriptors, D3D12_RESOURCE_DESC &&description, const std::wstring &fileName) {
    this->cpuDescriptors = std::move(cpuDescriptors);
    this->description = std::move(description);
    this->fileName = fileName;
}

#include "TextureImpl.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "CommandList/CommandQueue.h"
#include "Resource/ConstantBuffers.h"
#include "Utility/FileHelper.h"
#include "Utility/ThrowIfFailed.h"

#include <DXD/ExternalHeadersWrappers/DirectXMath.h>
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

template <typename T>
inline T divideByMultiple(T value, size_t alignment) {
    // TODO move to same shared header
    return (T)((value + alignment - 1) / alignment);
}

TextureGpuLoadResult TextureImpl::gpuLoad(const TextureGpuLoadArgs &args) {
    TextureGpuLoadResult result = {};

    // Create GPU resource
    result.description = TextureImpl::createTextureDescription(args.metadata);
    result.description.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    this->setResource(createResource(ApplicationImpl::getInstance().getDevice(), &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
                                     &result.description, D3D12_RESOURCE_STATE_COPY_DEST, nullptr),
                      D3D12_RESOURCE_STATE_COPY_DEST, result.description.MipLevels);

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
    srvDescription.Texture2D.MipLevels = result.description.MipLevels;
    srvDescription.Texture2D.MostDetailedMip = 0;
    srvDescription.Texture2D.PlaneSlice = 0;
    srvDescription.Texture2D.ResourceMinLODClamp = 0;
    this->createSrv(&srvDescription);

    if (result.description.MipLevels > 1u) {
        assert(result.description.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D);
        generateMips(result.description);
    }

    return std::move(result);
}

void TextureImpl::generateMips(D3D12_RESOURCE_DESC description) {
    // Create command list
    auto &queue = ApplicationImpl::getInstance().getDirectCommandQueue();
    CommandList commandList{queue};
    commandList.setPipelineStateAndComputeRootSignature(PipelineStateController::Identifier::PIPELINE_STATE_GENERATE_MIPS);

    // Space for cpu visible descriptors, can be reused after committing to gpu visible heap (this is made on dispatch)
    auto cpuDescriptors = ApplicationImpl::getInstance().getDescriptorController().allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2);

    // Generate subsequent mips
    uint32_t currentSourceWidth = static_cast<uint32_t>(description.Width);
    uint32_t currentSourceHeight = static_cast<uint32_t>(description.Height);
    for (auto sourceMipLevel = 0u; sourceMipLevel < description.MipLevels - 1; sourceMipLevel++) {
        const auto outputMipsCount = 1u;    // how many mips shader will generate in the current dispatch
        const auto maxOutputMipsCount = 1u; // how many mips can a shader generate in one dispatch

        // Create descriptors and transition resources
        createDescriptorsForMipMapGeneration(cpuDescriptors, getResource(), description.Format, sourceMipLevel, outputMipsCount, maxOutputMipsCount);
        transitionSubresourcesForMipMapGeneration(commandList, sourceMipLevel, outputMipsCount);

        // Bind resources
        const uint32_t dstWidth = currentSourceWidth / 2;
        const uint32_t dstHeight = currentSourceHeight / 2;
        GenerateMipsCB cb = {};
        cb.texelSize = XMFLOAT2{1.0f / dstWidth, 1.0f / dstHeight};
        cb.sourceMipLevel = sourceMipLevel;
        commandList.setCbvSrvUavInDescriptorTable(0, 0, *this, cpuDescriptors.getCpuHandle(0));
        commandList.setCbvSrvUavInDescriptorTable(0, 1, *this, cpuDescriptors.getCpuHandle(1));
        commandList.setRoot32BitConstant(1, cb);

        // Dispatch
        const UINT threadGroupSize = 16u;
        const UINT threadGroupCountX = divideByMultiple(dstWidth, threadGroupSize);
        const UINT threadGroupCountY = divideByMultiple(dstHeight, threadGroupSize);
        commandList.dispatch(threadGroupCountX, threadGroupCountY, 1u);

        // Size of source texture in next iteration will be two times smaller, but cannot be 0
        currentSourceWidth = std::max(currentSourceWidth / 2, 1u);
        currentSourceHeight = std::max(currentSourceHeight / 2, 1u);
    }

    // Barriers
    commandList.uavBarrier(*this);
    commandList.transitionBarrier(*this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // Submit to gpu
    commandList.close();
    this->waitOnGpuForGpuUpload(queue);
    queue.executeCommandListAndSignal(commandList);
    queue.flush(true);
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
    textureDesc.MipLevels = computeMaxMipsCount(metadata.width, metadata.height);
    textureDesc.MipLevels = std::min(static_cast<UINT16>(Resource::maxSubresourcesCount), textureDesc.MipLevels);
    return textureDesc;
}

uint16_t TextureImpl::computeMaxMipsCount(size_t width, size_t height) {
    DWORD result;
    const DWORD biggerDimension = static_cast<DWORD>(std::max(width, height));
    const BOOLEAN success = _BitScanReverse(&result, biggerDimension);
    assert(success);
    return static_cast<uint16_t>(result);
}

void TextureImpl::createDescriptorsForMipMapGeneration(DescriptorAllocation &descriptorAllocation, ID3D12ResourcePtr resource,
                                                       DXGI_FORMAT format, uint32_t sourceMip, uint32_t outputMipsCount, uint32_t maxOutputMipsCount) {
    auto device = ApplicationImpl::getInstance().getDevice();

    // SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDescription = {};
    srvDescription.Format = format;
    srvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDescription.Texture2D.MipLevels = 1;
    srvDescription.Texture2D.MostDetailedMip = sourceMip;
    srvDescription.Texture2D.PlaneSlice = 0;
    srvDescription.Texture2D.ResourceMinLODClamp = 0;
    device->CreateShaderResourceView(resource.Get(), &srvDescription, descriptorAllocation.getCpuHandle(0));

    // Real UAVs
    for (auto i = 0u; i < outputMipsCount; i++) {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescription = {};
        uavDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO
        uavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDescription.Texture2D.MipSlice = sourceMip + i + 1;
        uavDescription.Texture2D.PlaneSlice = 0;
        device->CreateUnorderedAccessView(resource.Get(), nullptr, &uavDescription, descriptorAllocation.getCpuHandle(i + 1));
    }

    // Null UAVs, required by the runtime to be bound
    for (auto i = outputMipsCount; i < maxOutputMipsCount; i++) {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;
        uavDesc.Texture2D.PlaneSlice = 0;
        device->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, descriptorAllocation.getCpuHandle(i + 1));
    }
}

void TextureImpl::transitionSubresourcesForMipMapGeneration(CommandList &commandList, uint32_t sourceMip, uint32_t outputMipsCount) {
    commandList.transitionBarrier(*this, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    for (auto i = 0u; i < outputMipsCount; i++) {
        const auto subresourceIndex = sourceMip + i + 1;
        commandList.transitionBarrier(*this, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, subresourceIndex);
    }
}

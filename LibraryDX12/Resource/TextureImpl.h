#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "DirectXTex/DirectXTex/DirectXTex.h"
#include "Resource/Resource.h"
#include "Threading/AsyncLoadableObject.h"

#include "DXD/Texture.h"

#include <DXD/ExternalHeadersWrappers/d3d12.h>
#include <atomic>

class ApplicationImpl;

struct TextureCpuLoadArgs {
    const std::wstring filePath;
};

struct TextureCpuLoadResult {
    bool success = false;
    DirectX::TexMetadata metadata = {};
    DirectX::ScratchImage scratchImage = {};
};

struct TextureGpuLoadArgs {
    const DirectX::TexMetadata &metadata;
    const DirectX::ScratchImage &scratchImage;
};

using TextureAsyncLoadableObject = AsyncLoadableObject<TextureCpuLoadArgs, TextureCpuLoadResult, TextureGpuLoadArgs>;

// TODO if texture used as Albedo (Diffuse), make SRGB, e.g. convert DXGI_FORMAT_R8G8B8A8_UNORM to DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
// TODO mips
class TextureImpl : public DXD::Texture, public Resource, public TextureAsyncLoadableObject {
protected:
    friend class DXD::Texture;
    TextureImpl(const std::wstring &filePath, bool asynchronousLoading);
    ~TextureImpl() override;

private:
    // Helpers
    static D3D12_RESOURCE_DESC createTextureDescription(const DirectX::TexMetadata &metadata);
    static uint16_t computeMaxMipsCount(size_t width, size_t height);
    void generateMips();
    static void createDescriptorsForMipMapGeneration(DescriptorAllocation &descriptorAllocation, ID3D12ResourcePtr resource, DXGI_FORMAT format,
                                                     uint32_t sourceMip, uint32_t outputMipsCount, uint32_t maxOutputMipsCount);
    void transitionSubresourcesForMipMapGeneration(CommandList &commandList, uint32_t sourceMip, uint32_t outputMipsCount);

    // AsyncLoadableObject overrides
    TextureCpuLoadResult cpuLoad(const TextureCpuLoadArgs &args) override;
    bool isCpuLoadSuccessful(const TextureCpuLoadResult &result) override;
    TextureGpuLoadArgs createArgsForGpuLoad(const TextureCpuLoadResult &cpuLoadResult) override;
    void gpuLoad(const TextureGpuLoadArgs &args) override;
    bool hasGpuLoadEnded() override;

    // Fields
    D3D12_RESOURCE_DESC description = {};
    std::wstring fileName = {};
};

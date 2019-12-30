#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "DirectXTex/DirectXTex/DirectXTex.h"
#include "Resource/Resource.h"
#include "Threading/CpuGpuOperation.h"

#include <DXD/ExternalHeadersWrappers/d3d12.h>
#include <DXD/Texture.h>
#include <atomic>

class ApplicationImpl;

class TextureImpl : public DXD::Texture, public Resource {
public:
    bool isReady();

protected:
    friend class DXD::Texture;
    TextureImpl(const std::wstring &filePath, DXD::Texture::TextureType type, TextureLoadEvent *loadEvent);
    TextureImpl(const std::wstring &filePath, DXD::Texture::TextureType type, TextureLoadResult *loadResult);
    ~TextureImpl() override;

private:
    // Helpers
    static D3D12_RESOURCE_DESC createTextureDescription(const DirectX::TexMetadata &metadata);
    static uint16_t computeMaxMipsCount(size_t width, size_t height);
    void generateMips();
    static void createDescriptorsForMipMapGeneration(DescriptorAllocation &descriptorAllocation, ID3D12ResourcePtr resource, DXGI_FORMAT format,
                                                     uint32_t sourceMip, uint32_t outputMipsCount, uint32_t maxOutputMipsCount);
    void transitionSubresourcesForMipMapGeneration(CommandList &commandList, uint32_t sourceMip, uint32_t outputMipsCount);

    // Loading
    struct TextureCpuLoadArgs {
        const std::wstring filePath;
        DXD::Texture::TextureType type;
    };

    struct TextureCpuLoadResult {
        DXD::Texture::TextureLoadResult result = {};
        DirectX::TexMetadata metadata = {};
        DirectX::ScratchImage scratchImage = {};
    };

    class TextureLoadCpuGpuOperation : public CpuGpuOperation<TextureCpuLoadArgs, TextureCpuLoadResult, DXD::Texture::TextureLoadResult> {
    public:
        TextureLoadCpuGpuOperation(TextureImpl &texture) : texture(texture) {}

    protected:
        TextureCpuLoadResult cpuLoad(const TextureCpuLoadArgs &args) override;
        bool isCpuLoadSuccessful(const TextureCpuLoadResult &cpuLoadResult) override;
        void gpuLoad(const TextureCpuLoadResult &args) override;
        bool hasGpuLoadEnded() override;
        DXD::Texture::TextureLoadResult getOperationResult(const TextureCpuLoadResult &cpuLoadResult) const override;

    private:
        TextureImpl &texture;
    };

    // Fields
    TextureLoadCpuGpuOperation loadOperation;
    D3D12_RESOURCE_DESC description = {};
    std::wstring fileName = {};
};

#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "DirectXTex/DirectXTex/DirectXTex.h"
#include "Resource/Resource.h"
#include "Threading/AsyncLoadableObject.h"

#include "DXD/Texture.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
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

struct TextureGpuLoadResult {
    D3D12_RESOURCE_DESC description;
};

using TextureAsyncLoadableObject = AsyncLoadableObject<TextureCpuLoadArgs, TextureCpuLoadResult, TextureGpuLoadArgs, TextureGpuLoadResult>;

// TODO if texture used as Albedo (Diffuse), make SRGB, e.g. convert DXGI_FORMAT_R8G8B8A8_UNORM to DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
// TODO mips
class TextureImpl : public DXD::Texture, public Resource, TextureAsyncLoadableObject {
protected:
    friend class DXD::Texture;
    TextureImpl(ApplicationImpl &application, const std::wstring &filePath, bool asynchronousLoading);
    ~TextureImpl() override;

public:
    // Accessors
    bool isUploadInProgress() override;

private:
    // Helpers
    static D3D12_RESOURCE_DESC createTextureDescription(const DirectX::TexMetadata &metadata);

    // AsyncLoadableObject overrides
    TextureCpuLoadResult cpuLoad(const TextureCpuLoadArgs &args) override;
    bool isCpuLoadSuccessful(const TextureCpuLoadResult &result) override;
    TextureGpuLoadArgs createArgsForGpuLoad(const TextureCpuLoadResult &cpuLoadResult) override;
    TextureGpuLoadResult gpuLoad(const TextureGpuLoadArgs &args) override;
    void writeCpuGpuLoadResults(TextureCpuLoadResult &cpuLoadResult, TextureGpuLoadResult &gpuLoadResult) override;

    // Fields
    D3D12_RESOURCE_DESC description = {};
    std::wstring fileName = {};
};

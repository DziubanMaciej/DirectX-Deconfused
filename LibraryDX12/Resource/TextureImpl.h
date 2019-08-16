#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "DirectXTex/DirectXTex/DirectXTex.h"
#include "Resource/Resource.h"

#include "DXD/Texture.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <atomic>

class ApplicationImpl;

// TODO if texture used as Albedo (Diffuse), make SRGB, e.g. convert DXGI_FORMAT_R8G8B8A8_UNORM to DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
// TODO mips
class TextureImpl : public DXD::Texture, public Resource {
protected:
    friend class DXD::Texture;
    TextureImpl(ApplicationImpl &application, const std::wstring &filePath);
    TextureImpl(ApplicationImpl &application, const D3D12_RESOURCE_DESC &description, const std::wstring &fileName, const DirectX::ScratchImage &image);

public:
    D3D12_CPU_DESCRIPTOR_HANDLE getSrvDescriptor() const { return cpuDescriptors.getCpuHandle(0); }

private:
    static D3D12_RESOURCE_DIMENSION calculateTextureDimension(int width, int height);
    static D3D12_RESOURCE_DESC createTextureDescription(const DirectX::TexMetadata &metadata);
    void loadAndUpload(ApplicationImpl &application, const std::wstring &filePath);

    DescriptorAllocation cpuDescriptors = {};
    D3D12_RESOURCE_DESC description = {};
    std::wstring fileName = {};
    std::atomic_bool loadingComplete = false;

    // Loading texture on CPU
    struct LoadResults {
        bool success = false;
        DirectX::TexMetadata metadata = {};
        DirectX::ScratchImage scratchImage = {};
    };
    LoadResults loadOnCpu(ApplicationImpl &application, const std::wstring &filePath);

    // Setting all the data after loading and uploading to the GPU
    void setData(DescriptorAllocation &&cpuDescriptors, D3D12_RESOURCE_DESC &&description, const std::wstring &fileName);
};

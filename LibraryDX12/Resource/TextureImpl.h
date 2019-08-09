#pragma once

#include "Descriptor/CpuDescriptorAllocation.h"
#include "DirectXTex/DirectXTex/DirectXTex.h"
#include "Resource/Resource.h"

#include "DXD/Texture.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"

class ApplicationImpl;

class TextureImpl : public DXD::Texture, public Resource {
protected:
    friend class DXD::Texture;
    TextureImpl(ApplicationImpl &application, const D3D12_RESOURCE_DESC &description, const std::string &fileName, const DirectX::ScratchImage &image);

public:
    D3D12_CPU_DESCRIPTOR_HANDLE getSrvDescriptor() const { return cpuDescriptors.getCpuHandle(0); }

private:
    static D3D12_RESOURCE_DIMENSION calculateTextureDimension(int width, int height);
    static D3D12_RESOURCE_DESC createTextureDescription(const DirectX::TexMetadata &metadata);

    CpuDescriptorAllocation cpuDescriptors;
    const D3D12_RESOURCE_DESC description;
    const std::string &fileName;
};

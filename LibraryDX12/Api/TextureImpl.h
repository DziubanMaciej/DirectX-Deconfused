#pragma once

#include "DXD/Texture.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"

class ApplicationImpl;

class TextureImpl : public DXD::Texture {
protected:
    friend class DXD::Texture;
    TextureImpl(ID3D12ResourcePtr resource, const std::string &fileName, D3D12_RESOURCE_DIMENSION dimension);

public:
private:
    static D3D12_RESOURCE_DIMENSION calculateTextureDimension(int width, int height);
    static D3D12_RESOURCE_DESC createTextureDescription(D3D12_RESOURCE_DIMENSION dimension, DXGI_FORMAT format, int width, int height);
    static ID3D12ResourcePtr createAndUploadResource(ApplicationImpl &aplication, unsigned char *imageData, const D3D12_RESOURCE_DESC &textureDescription);

    ID3D12ResourcePtr resource;
    const D3D12_RESOURCE_DIMENSION dimension;
    const std::string &fileName;
};

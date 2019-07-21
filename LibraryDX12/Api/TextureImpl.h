#pragma once

#include "Wrappers/Resource.h"

#include "DXD/Texture.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"

class ApplicationImpl;

class TextureImpl : public DXD::Texture, public Resource {
protected:
    friend class DXD::Texture;
    TextureImpl(ApplicationImpl &application, const D3D12_RESOURCE_DESC &description, const std::string &fileName, unsigned char *imageData);

public:
private:
    static D3D12_RESOURCE_DIMENSION calculateTextureDimension(int width, int height);
    static D3D12_RESOURCE_DESC createTextureDescription(D3D12_RESOURCE_DIMENSION dimension, DXGI_FORMAT format, int width, int height);
    void upload(ApplicationImpl &aplication, unsigned char *imageData, const D3D12_RESOURCE_DESC &textureDescription);

    const D3D12_RESOURCE_DESC description;
    const std::string &fileName;
};

#pragma once

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/d2d.h>
#include <DXD/ExternalHeadersWrappers/d3d12.h>

class Resource;
class AcquiredD2DWrappedResource;

class D2DWrappedResource : DXD::NonCopyableAndMovable {
public:
    D2DWrappedResource(Resource &d12Resource, D3D12_RESOURCE_STATES inState, D3D12_RESOURCE_STATES outState)
        : d12Resource(d12Resource),
          inState(inState),
          outState(outState) {}

    void wrap(float dpi);
    void reset();
    AcquiredD2DWrappedResource acquire();

private:
    // Constant data
    Resource &d12Resource;
    const D3D12_RESOURCE_STATES inState;
    const D3D12_RESOURCE_STATES outState;

    // Buffer which will be created and destroyed
    ID3D11ResourcePtr d3d11Resource = {};
    ID2D1BitmapPtr d2dResource = {};

    // Acquired flag, should be set only by AcquiredD2DWrappedResource
    friend AcquiredD2DWrappedResource;
    bool acquired = false;
};

class AcquiredD2DWrappedResource : DXD::NonCopyable {
public:
    AcquiredD2DWrappedResource(D2DWrappedResource &parent);
    AcquiredD2DWrappedResource(AcquiredD2DWrappedResource &&other);
    AcquiredD2DWrappedResource &operator=(AcquiredD2DWrappedResource &&other) = delete;
    ~AcquiredD2DWrappedResource();

    auto &getD3D11Resource() { return parent->d3d11Resource; }
    auto &getD2DResource() { return parent->d2dResource; }

private:
    D2DWrappedResource *parent;
};

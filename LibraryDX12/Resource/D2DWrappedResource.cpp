#include "D2DWrappedResource.h"

#include "Application/ApplicationImpl.h"
#include "Resource/Resource.h"
#include "Utility/ThrowIfFailed.h"

#include <cassert>

void D2DWrappedResource::wrap(float dpi) {
    auto &d2dContext = ApplicationImpl::getInstance().getD2DContext();

    // Create DX11 Resource
    const D3D11_RESOURCE_FLAGS d3d11Flags = {D3D11_BIND_RENDER_TARGET};
    throwIfFailed(d2dContext.getD3D11On12Device()->CreateWrappedResource(
        d12Resource.getResource().Get(), &d3d11Flags,
        inState, outState,
        IID_PPV_ARGS(&d3d11Resource)));

    // Convert to DXGI surface
    IDXGISurfacePtr surface = {};
    throwIfFailed(d3d11Resource.As(&surface));

    // Create D2D Resource
    const auto bitmapProperties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
        dpi, dpi);
    throwIfFailed(d2dContext.getD2DDeviceContext()->CreateBitmapFromDxgiSurface(
        surface.Get(),
        &bitmapProperties,
        &d2dResource));
}

void D2DWrappedResource::reset() {
    d3d11Resource.Reset();
    d2dResource.Reset();
}

AcquiredD2DWrappedResource D2DWrappedResource::acquire() {
    assert(!this->acquired);
    return AcquiredD2DWrappedResource{*this};
}

inline AcquiredD2DWrappedResource::AcquiredD2DWrappedResource(D2DWrappedResource &parent) : parent(&parent) {
    assert(!parent.acquired);

    auto &device = ApplicationImpl::getInstance().getD2DContext().getD3D11On12Device();
    device->AcquireWrappedResources(parent.d3d11Resource.GetAddressOf(), 1);
    parent.acquired = true;
}

AcquiredD2DWrappedResource::AcquiredD2DWrappedResource(AcquiredD2DWrappedResource &&other) : parent(other.parent) {
    other.parent = nullptr;
}

AcquiredD2DWrappedResource::~AcquiredD2DWrappedResource() {
    if (parent == nullptr) {
        return;
    }
    assert(parent->acquired);

    // Release
    parent->acquired = false;
    auto &device = ApplicationImpl::getInstance().getD2DContext().getD3D11On12Device();
    device->ReleaseWrappedResources(parent->d3d11Resource.GetAddressOf(), 1); // This call makes implicit state transition

    // Manage state
    assert(parent->d12Resource.getState() == parent->inState);
    parent->d12Resource.setState(Resource::ResourceState{parent->outState});

    // Flush to submit the D2D command list to the shared command queue.
    ApplicationImpl::getInstance().getD2DContext().getD3D11DeviceContext()->Flush();
}

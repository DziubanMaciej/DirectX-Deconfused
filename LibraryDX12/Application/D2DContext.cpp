#include "D2DContext.h"

#include "Utility/ThrowIfFailed.h"

#include "DXD/ExternalHeadersWrappers/dxgi.h"

D2DContext::D2DContext(ID3D12DevicePtr device, ID3D12CommandQueuePtr queue) {
    // Create DX11 device wrapped around the DX12 device with shared CommandQueue
    ID3D11DevicePtr d3d11Device = {};
    throwIfFailed(D3D11On12CreateDevice(
        device.Get(),
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr,
        0,
        reinterpret_cast<IUnknown **>(queue.GetAddressOf()),
        1,
        0,
        &d3d11Device,
        &d3d11DeviceContext,
        nullptr));
    throwIfFailed(d3d11Device.As(&d3d11On12Device));

    // Create D2D Factory
    D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};
    throwIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                    __uuidof(ID2D1Factory3), &d2dFactoryOptions, &d2dFactory));

    // Create D2D Device
    IDXGIDevicePtr dxgiDevice = {};
    throwIfFailed(d3d11On12Device.As(&dxgiDevice));
    throwIfFailed(d2dFactory->CreateDevice(dxgiDevice.Get(), &d2dDevice));

    // Create D2D Device context
    const D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
    throwIfFailed(d2dDevice->CreateDeviceContext(deviceOptions, &d2dDeviceContext));

    // Create DWrite Factory
    throwIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &dWriteFactory));
}

void D2DContext::wrapRenderTargetD11(ID3D12ResourcePtr &dx12Resource, ID3D11ResourcePtr &dx11Resource, D3D12_RESOURCE_STATES inState, D3D12_RESOURCE_STATES outState) {
    const D3D11_RESOURCE_FLAGS d3d11Flags = {D3D11_BIND_RENDER_TARGET};
    throwIfFailed(d3d11On12Device->CreateWrappedResource(
        dx12Resource.Get(), &d3d11Flags,
        inState, outState,
        IID_PPV_ARGS(&dx11Resource)));
}

void D2DContext::wrapRenderTargetD2D(ID3D11ResourcePtr &dx11Resource, ID2D1BitmapPtr &d2dResource, UINT dpi) {
    // Convert to DXGI surface
    IDXGISurfacePtr surface = {};
    throwIfFailed(dx11Resource.As(&surface));

    // Wrap the buffer
    const auto bitmapProperties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
        static_cast<float>(dpi), static_cast<float>(dpi));
    throwIfFailed(d2dDeviceContext->CreateBitmapFromDxgiSurface(
        surface.Get(),
        &bitmapProperties,
        &d2dResource));
}

void D2DContext::flushAndResetTarget() {
    d2dDeviceContext->SetTarget(nullptr);
    d3d11DeviceContext->Flush();
}

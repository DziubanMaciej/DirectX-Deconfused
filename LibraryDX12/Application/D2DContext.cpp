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

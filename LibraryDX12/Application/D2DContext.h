#pragma once

#include <DXD/ExternalHeadersWrappers/d2d.h>
#include <DXD/ExternalHeadersWrappers/d3d12.h>

class D2DContext {
public:
    D2DContext(ID3D12DevicePtr device, ID3D12CommandQueuePtr queue);

    auto &getD3D11DeviceContext() { return d3d11DeviceContext; }
    auto &getD2DDeviceContext() { return d2dDeviceContext; }
    auto &getD3D11On12Device() { return d3d11On12Device; }
    auto &getDWriteFactory() { return dWriteFactory; }

private:
    // D3D11 Context
    ID3D11DeviceContextPtr d3d11DeviceContext;
    ID3D11On12DevicePtr d3d11On12Device;

    // D2D Context
    ID2D1FactoryPtr d2dFactory;
    ID2D1DevicePtr d2dDevice;
    ID2D1DeviceContextPtr d2dDeviceContext;

    // DWrite
    IDWriteFactoryPtr dWriteFactory;
};

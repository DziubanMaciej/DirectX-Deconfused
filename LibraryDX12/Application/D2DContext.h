#pragma once

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <d2d1_3.h>
#include <d3d11.h>
#include <d3d11on12.h>
#include <dwrite.h>
#include <wrl.h>

class D2DContext {
public:
    D2DContext(ID3D12DevicePtr device, ID3D12CommandQueuePtr queue);

    void wrapRenderTargetD11(ID3D12ResourcePtr &dx12Resource,
                             Microsoft::WRL::ComPtr<ID3D11Resource> &dx11Resource,
                             D3D12_RESOURCE_STATES inState, D3D12_RESOURCE_STATES outState);
    void wrapRenderTargetD2D(Microsoft::WRL::ComPtr<ID3D11Resource> &dx11Resource,
                             Microsoft::WRL::ComPtr<ID2D1Bitmap1> &d2dResource, UINT dpi);
    void flushAndResetTarget();

    auto &getD3D11DeviceContext() { return d3d11DeviceContext; }
    auto &getD2DDeviceContext() { return d2dDeviceContext; }
    auto &getD3D11On12Device() { return d3d11On12Device; }
    auto &getDWriteFactory() { return dWriteFactory; }

private:
    // D3D11 Context
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
    Microsoft::WRL::ComPtr<ID3D11On12Device> d3d11On12Device;

    // D2D Context
    Microsoft::WRL::ComPtr<ID2D1Factory3> d2dFactory;
    Microsoft::WRL::ComPtr<ID2D1Device2> d2dDevice;
    Microsoft::WRL::ComPtr<ID2D1DeviceContext2> d2dDeviceContext;

    // DWrite
    Microsoft::WRL::ComPtr<IDWriteFactory> dWriteFactory;
};

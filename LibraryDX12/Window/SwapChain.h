#pragma once

#include "Application/SettingsImpl.h"
#include "Descriptor/DescriptorAllocation.h"
#include "Resource/RenderTarget.h"

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include "DXD/ExternalHeadersWrappers/dxgi.h"
#include <memory>
#include <stdint.h>
#include <vector>

#include <d3d11on12.h>
#include <d3d11.h>
#include <d2d1_3.h>
#include <dwrite.h>

class CommandQueue;
class DescriptorController;

class SwapChain : DXD::NonCopyableAndMovable {
    struct BackBufferEntry {
        RenderTarget backBuffer{nullptr, D3D12_RESOURCE_STATE_PRESENT};
        uint64_t lastFence;
        Microsoft::WRL::ComPtr<ID3D11Resource> m_wrappedBackBuffers;
        Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_d2dRenderTargets;
    };

public:
    SwapChain(HWND windowHandle, CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);
    void present(uint64_t fenceValue);
    void resize(int desiredWidth, int desiredHeight);

    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }

    uint64_t getFenceValueForCurrentBackBuffer() const;
    auto &getCurrentBackBuffer() { return backBufferEntries[this->currentBackBufferIndex].backBuffer; };
    auto &getCurrentD11BackBuffer() { return backBufferEntries[this->currentBackBufferIndex].m_wrappedBackBuffers; };
    auto &getCurrentD2DBackBuffer() { return backBufferEntries[this->currentBackBufferIndex].m_d2dRenderTargets; };
    IDXGISwapChainPtr swapChain;

private:
    static bool checkTearingSupport(IDXGIFactoryPtr &factory);
    static IDXGISwapChainPtr createSwapChain(HWND hwnd, IDXGIFactoryPtr &factory, CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);

    void resetRenderTargetViews();
    void updateRenderTargetViews();
    void resizeRenderTargets(uint32_t desiredWidth, uint32_t desiredHeight);

    // DX12 data
    std::vector<BackBufferEntry> backBufferEntries;

    //DX11 data
    D2D1_BITMAP_PROPERTIES1 bitmapProperties;

    // Numerical data
    uint32_t width;
    uint32_t height;
    UINT currentBackBufferIndex;
};

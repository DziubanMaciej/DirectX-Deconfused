#pragma once

#include "Application/SettingsImpl.h"
#include "Descriptor/DescriptorAllocation.h"
#include "Resource/D2DWrappedResource.h"
#include "Resource/Resource.h"

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/d2d.h>
#include <DXD/ExternalHeadersWrappers/d3d12.h>
#include <DXD/ExternalHeadersWrappers/dxgi.h>
#include <memory>
#include <stdint.h>
#include <vector>

class CommandQueue;
class DescriptorController;

class SwapChain : DXD::NonCopyableAndMovable {
    struct BackBufferEntry {
        Resource backBuffer{nullptr, D3D12_RESOURCE_STATE_PRESENT};
        D2DWrappedResource d2dWrappedBackBuffer{backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT};
        uint64_t lastFence;
    };

public:
    SwapChain(HWND windowHandle, CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);
    void present(uint64_t fenceValue);
    void resize(int desiredWidth, int desiredHeight);

    float getWidth() const { return static_cast<float>(width); }
    float getHeight() const { return static_cast<float>(height); }
    uint32_t getWidthUint() const { return width; }
    uint32_t getHeightUint() const { return height; }

    uint64_t getFenceValueForCurrentBackBuffer() const;
    auto &getCurrentBackBuffer() { return backBufferEntries[this->currentBackBufferIndex].backBuffer; };
    D2DWrappedResource &getCurrentD2DWrappedBackBuffer();

private:
    static bool checkTearingSupport(IDXGIFactoryPtr &factory);
    static IDXGISwapChainPtr createSwapChain(HWND hwnd, IDXGIFactoryPtr &factory, CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);

    void resetRenderTargetViews();
    void resizeRenderTargets(uint32_t desiredWidth, uint32_t desiredHeight);
    void updateRenderTargetViews();
    void updateD2DWrappedBackBuffers();

    // DX12 data
    std::vector<BackBufferEntry> backBufferEntries;
    IDXGISwapChainPtr swapChain;

    //DX11 data
    D2D1_BITMAP_PROPERTIES1 bitmapProperties;

    // Numerical data
    const UINT windowDpi;
    uint32_t width;
    uint32_t height;
    UINT currentBackBufferIndex;
    bool initializedD2D = false;
};

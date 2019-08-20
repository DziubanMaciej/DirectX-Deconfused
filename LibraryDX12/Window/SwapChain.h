#pragma once

#include "Application/SettingsImpl.h"
#include "Descriptor/DescriptorAllocation.h"
#include "Resource/Resource.h"

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include "DXD/ExternalHeadersWrappers/dxgi.h"
#include <memory>
#include <stdint.h>
#include <vector>

class CommandQueue;
class DescriptorController;

class SwapChain : DXD::NonCopyableAndMovable {
    struct BackBufferEntry {
        std::unique_ptr<Resource> backBuffer = std::make_unique<Resource>(nullptr, D3D12_RESOURCE_STATE_PRESENT);
        uint64_t lastFence;
    };

public:
    SwapChain(HWND windowHandle, CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);
    void present(uint64_t fenceValue);
    void resize(int desiredWidth, int desiredHeight);

    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }

    uint64_t getFenceValueForCurrentBackBuffer() const;
    D3D12_CPU_DESCRIPTOR_HANDLE getCurrentBackBufferDescriptor() const;
    auto &getCurrentBackBuffer() { return backBufferEntries[this->currentBackBufferIndex].backBuffer; };
private:
    static bool checkTearingSupport(IDXGIFactoryPtr &factory);
    static IDXGISwapChainPtr createSwapChain(HWND hwnd, IDXGIFactoryPtr &factory, CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);

    void resetRenderTargetViews();
    void updateRenderTargetViews();
    void resizeRenderTargets(uint32_t desiredWidth, uint32_t desiredHeight);

    // Wrapped object
    IDXGISwapChainPtr swapChain;

    // Back buffer data
    std::vector<BackBufferEntry> backBufferEntries;
    DescriptorAllocation backBufferRtvDescriptors;

    // Numerical data
    uint32_t width;
    uint32_t height;
    UINT currentBackBufferIndex;

};

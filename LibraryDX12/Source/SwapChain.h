#pragma once

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include "DXD/ExternalHeadersWrappers/dxgi.h"
#include <stdint.h>
#include <vector>

class CommandQueue;

class SwapChain {
    struct BackBufferEntry {
        ID3D12ResourcePtr backBuffer;
        uint64_t lastFence;
    };

public:
    SwapChain(HWND windowHandle, ID3D12DevicePtr device, IDXGIFactoryPtr factory, CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);
    void present(uint64_t fenceValue);
    void resize(int desiredWidth, int desiredHeight);

    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }

    uint64_t getFenceValueForCurrentBackBuffer() const;
    D3D12_CPU_DESCRIPTOR_HANDLE getCurrentBackBufferDescriptor() const;
    ID3D12ResourcePtr &getCurrentBackBuffer();
    D3D12_CPU_DESCRIPTOR_HANDLE getDepthStencilBufferDescriptor() const;

private:
    static bool checkTearingSupport(IDXGIFactoryPtr &factory);
    static IDXGISwapChainPtr createSwapChain(HWND hwnd, IDXGIFactoryPtr &factory, CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);
    static ID3D12DescriptorHeapPtr createRtvDescriptorHeap(ID3D12DevicePtr device, uint32_t bufferCount);
    static ID3D12DescriptorHeapPtr createDsvDescriptorHeap(ID3D12DevicePtr device);

    void resetRenderTargetViews();
    void updateRenderTargetViews();
    void updateDepthStencilBuffer(uint32_t desiredWidth, uint32_t desiredHeight);

    void resizeRenderTargets(uint32_t desiredWidth, uint32_t desiredHeight);

    IDXGISwapChainPtr swapChain;
    ID3D12DevicePtr device;

    ID3D12DescriptorHeapPtr rtvDescriptorHeap;
    ID3D12DescriptorHeapPtr depthStencilDescriptorHeap;
    std::vector<BackBufferEntry> backBufferEntries;
    ID3D12ResourcePtr depthStencilBuffer;

    uint32_t width;
    uint32_t height;
    size_t currentBackBufferIndex;
};

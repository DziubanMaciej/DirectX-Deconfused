#pragma once

#include "Descriptor/CpuDescriptorAllocation.h"
#include "Resource/Resource.h"

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include "DXD/ExternalHeadersWrappers/dxgi.h"
#include <memory>
#include <stdint.h>
#include <vector>

class CommandQueue;
class DescriptorManager;

class SwapChain : DXD::NonCopyableAndMovable {
    struct BackBufferEntry {
        std::unique_ptr<Resource> backBuffer;
        uint64_t lastFence;
    };

public:
    SwapChain(HWND windowHandle, ID3D12DevicePtr device, DescriptorManager &descriptorManager, IDXGIFactoryPtr factory,
              CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);
    void present(uint64_t fenceValue);
    void resize(int desiredWidth, int desiredHeight);

    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }

    uint64_t getFenceValueForCurrentBackBuffer() const;
    D3D12_CPU_DESCRIPTOR_HANDLE getCurrentBackBufferDescriptor() const;
    auto &getCurrentBackBuffer() { return backBufferEntries[this->currentBackBufferIndex].backBuffer; };
    D3D12_CPU_DESCRIPTOR_HANDLE getDepthStencilBufferDescriptor() const;
    auto &getDepthStencilBuffer() { return depthStencilBuffer; };

    const CpuDescriptorAllocation &getSrvDescriptor() const { return srvDescriptor; }
    const CpuDescriptorAllocation &getPostProcessRtvDescriptor() const { return postProcessRtvDescriptor; }
    const CpuDescriptorAllocation &getShadowMapDescriptor() const { return shadowMapDescriptor; }
    const CpuDescriptorAllocation &getShadowMapSrvDescriptor() const { return shadowMapSrvDescriptor; }

    auto &getPostProcessRenderTarget() { return postProcessRenderTarget; }
    auto &getShadowMap(int i) { return shadowMap[i]; }

private:
    static bool checkTearingSupport(IDXGIFactoryPtr &factory);
    static IDXGISwapChainPtr createSwapChain(HWND hwnd, IDXGIFactoryPtr &factory, CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);

    void resetRenderTargetViews();
    void updateRenderTargetViews();
    void updateDepthStencilBuffer(uint32_t desiredWidth, uint32_t desiredHeight);

    void resizeRenderTargets(uint32_t desiredWidth, uint32_t desiredHeight);

    // Base objects
    IDXGISwapChainPtr swapChain;
    ID3D12DevicePtr device;

    // Descriptors data
    DescriptorManager &descriptorManager;
    CpuDescriptorAllocation rtvDescriptors;
    CpuDescriptorAllocation dsvDescriptor;
    CpuDescriptorAllocation srvDescriptor;
    CpuDescriptorAllocation postProcessRtvDescriptor;
    CpuDescriptorAllocation shadowMapDescriptor;
    CpuDescriptorAllocation shadowMapSrvDescriptor;

    // Buffers
    std::vector<BackBufferEntry> backBufferEntries;
    std::unique_ptr<Resource> depthStencilBuffer;

    // Render Targets
    std::unique_ptr<Resource> postProcessRenderTarget;

    // Shadow Maps
    std::unique_ptr<Resource> shadowMap[8];

    // Numerical data
    uint32_t width;
    uint32_t height;
    UINT currentBackBufferIndex;
};

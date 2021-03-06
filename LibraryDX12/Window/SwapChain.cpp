#include "Swapchain.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandQueue.h"
#include "Utility/DxObjectNaming.h"
#include "Utility/ThrowIfFailed.h"

#include <ExternalHeaders/Wrappers/d3dx12.h>
#include <algorithm>

SwapChain::SwapChain(HWND windowHandle, CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount)
    : swapChain(createSwapChain(windowHandle, ApplicationImpl::getInstance().getFactory(), commandQueue, width, height, bufferCount)),
      backBufferEntries(bufferCount),
      width(width),
      height(height),
      currentBackBufferIndex(swapChain->GetCurrentBackBufferIndex()),
      windowDpi(GetDpiForWindow(windowHandle)) {
}

bool SwapChain::checkTearingSupport(IDXGIFactoryPtr &factory) {
    BOOL allowTearing = FALSE;
    HRESULT result = factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
    if (FAILED(result)) {
        return false;
    }
    return allowTearing == TRUE;
}

IDXGISwapChainPtr SwapChain::createSwapChain(HWND hwnd, IDXGIFactoryPtr &factory, CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount) {
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;

    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Stereo = FALSE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    desc.BufferCount = bufferCount;
    desc.Scaling = DXGI_SCALING_NONE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc.Flags = checkTearingSupport(factory) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    throwIfFailed(factory->CreateSwapChainForHwnd(commandQueue.getCommandQueue().Get(), hwnd,
                                                  &desc, nullptr,
                                                  nullptr, &swapChain1));
    throwIfFailed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));

    IDXGISwapChainPtr resultSwapChain;
    throwIfFailed(swapChain1.As(&resultSwapChain));
    return resultSwapChain;
}

void SwapChain::updateRenderTargetViews() {
    auto &application = ApplicationImpl::getInstance();
    auto &device = application.getDevice();
    const auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    for (auto i = 0u; i < backBufferEntries.size(); i++) {
        BackBufferEntry &backBufferEntry = backBufferEntries[i];

        // Get current back buffer
        ID3D12ResourcePtr backBuffer;
        throwIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
        SET_OBJECT_NAME(backBuffer, L"BackBuffer%d", i);

        // Update DX12 back buffer data
        backBufferEntry.backBuffer.setResource(backBuffer, D3D12_RESOURCE_STATE_PRESENT, 1u);
        backBufferEntry.backBuffer.createRtv(nullptr);
    }

    // Update DX11/D2D back buffer data
    if (initializedD2D) {
        updateD2DWrappedBackBuffers();
    }
}

void SwapChain::updateD2DWrappedBackBuffers() {
    for (auto i = 0u; i < backBufferEntries.size(); i++) {
        BackBufferEntry &backBufferEntry = backBufferEntries[i];
        backBufferEntry.d2dWrappedBackBuffer.wrap(static_cast<float>(windowDpi));
    }
}

void SwapChain::present(uint64_t fenceValue) {
    // Save fence associated with this back buffer to wait for it later
    backBufferEntries[currentBackBufferIndex].lastFence = fenceValue;

    // Present
    const UINT syncInterval = ApplicationImpl::getInstance().getSettings().getVerticalSyncEnabled() ? 1u : 0u;
    throwIfFailed(swapChain->Present(syncInterval, 0));

    // Move to next back buffer
    currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
}

void SwapChain::resize(int desiredWidth, int desiredHeight) {
    this->width = std::max(static_cast<uint32_t>(desiredWidth), 1u);
    this->height = std::max(static_cast<uint32_t>(desiredHeight), 1u);
    resetRenderTargetViews();           // release references to back buffers, set all fences to currentValue
    resizeRenderTargets(width, height); // reallocate buffers of appriopriate size
    updateRenderTargetViews();          // update rtv descriptors;
}

void SwapChain::resetRenderTargetViews() {
    for (auto &backBufferEntry : backBufferEntries) {
        backBufferEntry.backBuffer.reset();
        backBufferEntry.d2dWrappedBackBuffer.reset();
        backBufferEntry.lastFence = currentBackBufferIndex;
    }

    if (initializedD2D) {
        auto d2dContext = ApplicationImpl::getInstance().getD2DContext();
        d2dContext.getD2DDeviceContext()->SetTarget(nullptr);
        d2dContext.getD3D11DeviceContext()->Flush();
    }
}

void SwapChain::resizeRenderTargets(uint32_t desiredWidth, uint32_t desiredHeight) {

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    throwIfFailed(swapChain->GetDesc(&swapChainDesc));
    throwIfFailed(swapChain->ResizeBuffers(
        static_cast<UINT>(backBufferEntries.size()),
        width, height,
        swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));
    currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
}

uint64_t SwapChain::getFenceValueForCurrentBackBuffer() const {
    return backBufferEntries[this->currentBackBufferIndex].lastFence;
}

D2DWrappedResource &SwapChain::getCurrentD2DWrappedBackBuffer() {
    if (!initializedD2D && ApplicationImpl::getInstance().isD2DContextInitialized()) {
        updateD2DWrappedBackBuffers();
        this->initializedD2D = true;
    }
    return backBufferEntries[this->currentBackBufferIndex].d2dWrappedBackBuffer;
}

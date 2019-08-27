#include "Swapchain.h"

#include "Application/ApplicationImpl.h"
#include "CommandList/CommandQueue.h"
#include "Utility/ThrowIfFailed.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <algorithm>

SwapChain::SwapChain(HWND windowHandle, CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount)
    : swapChain(createSwapChain(windowHandle, ApplicationImpl::getInstance().getFactory(), commandQueue, width, height, bufferCount)),
      backBufferEntries(bufferCount),
      width(width),
      height(height),
      currentBackBufferIndex(swapChain->GetCurrentBackBufferIndex()) {
    // Query the desktop's dpi settings, which will be used to create
    // D2D's render targets.
    float dpiX;
    float dpiY;
    dpiX = dpiY = (float)GetDpiForWindow(windowHandle);
    this->bitmapProperties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
        dpiX,
        dpiY);
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
    auto device = ApplicationImpl::getInstance().getDevice();
    const auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (auto i = 0u; i < backBufferEntries.size(); i++) {
        ID3D12ResourcePtr backBuffer;
        throwIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        backBufferEntries[i].backBuffer.setResource(backBuffer);
        backBufferEntries[i].backBuffer.createRtv(nullptr);
        D3D11_RESOURCE_FLAGS d3d11Flags = {D3D11_BIND_RENDER_TARGET};
        throwIfFailed(ApplicationImpl::getInstance().m_d3d11On12Device->CreateWrappedResource(
            backBuffer.Get(),
            &d3d11Flags,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            IID_PPV_ARGS(&backBufferEntries[i].m_wrappedBackBuffers)));
        // Create a render target for D2D to draw directly to this back buffer.
        Microsoft::WRL::ComPtr<IDXGISurface> surface;
        throwIfFailed(backBufferEntries[i].m_wrappedBackBuffers.As(&surface));
        throwIfFailed(ApplicationImpl::getInstance().m_d2dDeviceContext->CreateBitmapFromDxgiSurface(
            surface.Get(),
            &bitmapProperties,
            &backBufferEntries[i].m_d2dRenderTargets));
    }
}

void SwapChain::present(uint64_t fenceValue) {
    // Save fence associated with this back buffer to wait for it later
    backBufferEntries[currentBackBufferIndex].lastFence = fenceValue;

    // Present
    UINT syncInterval = ApplicationImpl::getInstance().getSettings().getVerticalSyncEnabled() ? 1u : 0u;
    //UINT presentFlags = g_TearingSupported && !g_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    throwIfFailed(swapChain->Present(syncInterval, 0)); // TODO

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
        backBufferEntry.backBuffer.getResource().Reset();
        backBufferEntry.lastFence = currentBackBufferIndex;
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

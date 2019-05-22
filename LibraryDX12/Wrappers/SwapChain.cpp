#include "Swapchain.h"
#include "Wrappers/CommandQueue.h"
#include "Utility/ThrowIfFailed.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <algorithm>

SwapChain::SwapChain(HWND windowHandle, ID3D12DevicePtr device, IDXGIFactoryPtr factory,
                     CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount)
    : swapChain(createSwapChain(windowHandle, factory, commandQueue, width, height, bufferCount)),
      device(device),
      rtvDescriptorHeap(createRtvDescriptorHeap(device, bufferCount)),
      depthStencilDescriptorHeap(createDsvDescriptorHeap(device)),
      backBufferEntries(bufferCount),
      depthStencilBuffer(nullptr),
      width(width),
      height(height),
      currentBackBufferIndex(swapChain->GetCurrentBackBufferIndex()) {
    updateRenderTargetViews();
    updateDepthStencilBuffer(width, height);
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
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
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

ID3D12DescriptorHeapPtr SwapChain::createRtvDescriptorHeap(ID3D12DevicePtr device, uint32_t numDescriptors) {
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = numDescriptors;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    ID3D12DescriptorHeapPtr descriptorHeap;
    throwIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));
    return descriptorHeap;
}

ID3D12DescriptorHeapPtr SwapChain::createDsvDescriptorHeap(ID3D12DevicePtr device) {
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dsvHeapDesc.NodeMask = 1;

    ID3D12DescriptorHeapPtr descriptorHeap;
    throwIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&descriptorHeap)));
    return descriptorHeap;
}

void SwapChain::updateRenderTargetViews() {
    const auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    auto rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    for (auto i = 0u; i < backBufferEntries.size(); i++) {
        ID3D12ResourcePtr backBuffer;
        throwIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
        backBufferEntries[i].backBuffer = backBuffer;

        rtvHandle.ptr += rtvDescriptorSize;
    }
}

void SwapChain::present(uint64_t fenceValue) {
    // Save fence associated with this back buffer to wait for it later
    backBufferEntries[currentBackBufferIndex].lastFence = fenceValue;

    // Present
    //UINT syncInterval = g_VSync ? 1 : 0;
    //UINT presentFlags = g_TearingSupported && !g_VSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
    throwIfFailed(swapChain->Present(1u, 0)); // TODO

    // Move to next back buffer
    currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
}

void SwapChain::resize(int desiredWidth, int desiredHeight) {
    this->width = std::max(static_cast<uint32_t>(desiredWidth), 1u);
    this->height = std::max(static_cast<uint32_t>(desiredHeight), 1u);
    resetRenderTargetViews();                // release references to back buffers, set all fences to currentValue
    resizeRenderTargets(width, height);      // reallocate buffers of appriopriate size
    updateRenderTargetViews();               // update rtv descriptors;
    updateDepthStencilBuffer(width, height); // create new buffer and update descriptor on the heap
}

void SwapChain::resetRenderTargetViews() {
    for (auto &backBufferEntry : backBufferEntries) {
        backBufferEntry.backBuffer.Reset();
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

void SwapChain::updateDepthStencilBuffer(uint32_t desiredWidth, uint32_t desiredHeight) {
    D3D12_CLEAR_VALUE optimizedClearValue = {};
    optimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    optimizedClearValue.DepthStencil = {1.0f, 0};

    throwIfFailed(device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES{D3D12_HEAP_TYPE_DEFAULT},
        D3D12_HEAP_FLAG_NONE, // TODO D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES good?
        &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &optimizedClearValue,
        IID_PPV_ARGS(&depthStencilBuffer)));

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.Texture2D = D3D12_TEX2D_DSV{0};
    device->CreateDepthStencilView(
        depthStencilBuffer.Get(),
        &dsvDesc,
        depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

uint64_t SwapChain::getFenceValueForCurrentBackBuffer() const {
    return backBufferEntries[this->currentBackBufferIndex].lastFence;
}

ID3D12ResourcePtr &SwapChain::getCurrentBackBuffer() {
    return backBufferEntries[this->currentBackBufferIndex].backBuffer;
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::getDepthStencilBufferDescriptor() const {
    return depthStencilDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

ID3D12ResourcePtr &SwapChain::getDepthStencilBuffer() {
    return depthStencilBuffer;
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::getCurrentBackBufferDescriptor() const {
    const auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    return CD3DX12_CPU_DESCRIPTOR_HANDLE{
        rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
        static_cast<INT>(this->currentBackBufferIndex),
        rtvDescriptorSize};
}
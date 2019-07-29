#include "Swapchain.h"

#include "CommandList/CommandQueue.h"
#include "Descriptor/DescriptorManager.h"
#include "Utility/ThrowIfFailed.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <algorithm>

SwapChain::SwapChain(HWND windowHandle, ID3D12DevicePtr device, DescriptorManager &descriptorManager, IDXGIFactoryPtr factory,
                     CommandQueue &commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount)
    : swapChain(createSwapChain(windowHandle, factory, commandQueue, width, height, bufferCount)),
      device(device),
      descriptorManager(descriptorManager),
      rtvDescriptors(descriptorManager.allocate(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, bufferCount)),
      dsvDescriptor(descriptorManager.allocate(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1)),
      cbvDescriptorHeap(createCbvDescriptorHeap(device)),
      backBufferEntries(bufferCount),
      depthStencilBuffer(nullptr),
      simpleConstantBuffer(nullptr),
      width(width),
      height(height),
      currentBackBufferIndex(swapChain->GetCurrentBackBufferIndex()) {
    for (auto &entry : backBufferEntries) {
        entry.backBuffer = std::make_unique<Resource>(nullptr);
    }
    updateRenderTargetViews();
    updateDepthStencilBuffer(width, height);
    createSimpleConstantBuffer();
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

ID3D12DescriptorHeapPtr SwapChain::createCbvDescriptorHeap(ID3D12DevicePtr device) {
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    ID3D12DescriptorHeapPtr descriptorHeap;
    throwIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&descriptorHeap)));
    return descriptorHeap;
}

void SwapChain::updateRenderTargetViews() {
    const auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    auto rtvHandle = rtvDescriptors.getCpuHandle();
    for (auto i = 0u; i < backBufferEntries.size(); i++) {
        ID3D12ResourcePtr backBuffer;
        throwIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

        device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
        backBufferEntries[i].backBuffer->setResource(backBuffer);

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
        backBufferEntry.backBuffer->getResource().Reset();
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

    depthStencilBuffer = std::make_unique<Resource>(device,
                                                    &CD3DX12_HEAP_PROPERTIES{D3D12_HEAP_TYPE_DEFAULT},
                                                    D3D12_HEAP_FLAG_NONE, // TODO D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES good?
                                                    &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
                                                    D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                    &optimizedClearValue);

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.Texture2D = D3D12_TEX2D_DSV{0};
    device->CreateDepthStencilView(
        depthStencilBuffer->getResource().Get(),
        &dsvDesc,
        dsvDescriptor.getCpuHandle());
}

void SwapChain::createSimpleConstantBuffer() {

    simpleConstantBuffer = std::make_unique<Resource>(device,
                                                      &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                                                      D3D12_HEAP_FLAG_NONE,
                                                      &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
                                                      D3D12_RESOURCE_STATE_GENERIC_READ,
                                                      nullptr);

    // Describe and create a constant buffer view.
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = simpleConstantBuffer->getResource()->GetGPUVirtualAddress();
    cbvDesc.SizeInBytes = (sizeof(SimpleConstantBuffer) + 255) & ~255; // CB size is required to be 256-byte aligned.
    device->CreateConstantBufferView(&cbvDesc, cbvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // Map and initialize the constant buffer. We don't unmap this until the
    // app closes. Keeping things mapped for the lifetime of the resource is okay.
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    throwIfFailed(simpleConstantBuffer->getResource()->Map(0, &readRange, reinterpret_cast<void **>(&simpleCbvDataBegin)));
    memcpy(simpleCbvDataBegin, &simpleConstantBufferData, sizeof(simpleConstantBufferData));
}

uint64_t SwapChain::getFenceValueForCurrentBackBuffer() const {
    return backBufferEntries[this->currentBackBufferIndex].lastFence;
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::getDepthStencilBufferDescriptor() const {
    return dsvDescriptor.getCpuHandle();
}

SimpleConstantBuffer *SwapChain::getSimpleConstantBufferData() {
    return &simpleConstantBufferData;
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::getCurrentBackBufferDescriptor() const {
    return rtvDescriptors.getCpuHandle(currentBackBufferIndex);
}

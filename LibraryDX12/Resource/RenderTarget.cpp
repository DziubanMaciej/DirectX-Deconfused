#include "RenderTarget.h"

#include "Application/ApplicationImpl.h"
#include "Utility/ThrowIfFailed.h"

RenderTarget::RenderTarget(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES state)
    : Resource(resource, state),
      srv(ApplicationImpl::getInstance().getDescriptorController().allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1)),
      rtv(ApplicationImpl::getInstance().getDescriptorController().allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1)) {}

RenderTarget::RenderTarget(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE *pOptimizedClearValue)
    : Resource(device, pHeapProperties, heapFlags, pDesc, initialResourceState, pOptimizedClearValue),
      srv(ApplicationImpl::getInstance().getDescriptorController().allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1)),
      rtv(ApplicationImpl::getInstance().getDescriptorController().allocateCpu(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1)) {}

void RenderTarget::createSrv(D3D12_SHADER_RESOURCE_VIEW_DESC *desc) {
    ID3D12DevicePtr device = {};
    throwIfFailed(resource->GetDevice(IID_PPV_ARGS(&device)));
    device->CreateShaderResourceView(resource.Get(), desc, srv.getCpuHandle());
}

void RenderTarget::createRtv(D3D12_RENDER_TARGET_VIEW_DESC *desc) {
    ID3D12DevicePtr device = {};
    throwIfFailed(resource->GetDevice(IID_PPV_ARGS(&device)));
    device->CreateRenderTargetView(resource.Get(), desc, rtv.getCpuHandle());
}

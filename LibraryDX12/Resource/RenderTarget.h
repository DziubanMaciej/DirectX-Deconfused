#pragma once

#include "Resource/Resource.h"

class RenderTarget : public Resource {
public:
    /// Creates Resource object wrapping existing DX12 resource, used for back bufffers
    explicit RenderTarget(ID3D12ResourcePtr resource, D3D12_RESOURCE_STATES state);

    /// Allocates new resource
    explicit RenderTarget(ID3D12DevicePtr device, const D3D12_HEAP_PROPERTIES *pHeapProperties, D3D12_HEAP_FLAGS heapFlags,
                          const D3D12_RESOURCE_DESC *pDesc, D3D12_RESOURCE_STATES initialResourceState,
        const D3D12_CLEAR_VALUE *pOptimizedClearValue);

    void createSrv(D3D12_SHADER_RESOURCE_VIEW_DESC *desc);
    void createRtv(D3D12_RENDER_TARGET_VIEW_DESC *desc);

    D3D12_CPU_DESCRIPTOR_HANDLE getSrv() const { return srv.getCpuHandle(); }
    D3D12_CPU_DESCRIPTOR_HANDLE getRtv() const { return rtv.getCpuHandle(); }

private:
    DescriptorAllocation srv;
    DescriptorAllocation rtv;
};

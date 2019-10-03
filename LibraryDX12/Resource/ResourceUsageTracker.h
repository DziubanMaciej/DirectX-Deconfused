#pragma once

#include "Descriptor/DescriptorAllocation.h"

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/d3d12.h>
#include <map>
#include <set>
#include <vector>

class ResourceUsageTracker : DXD::NonCopyableAndMovable {
public:
    /// Notify the tracker about GPU accessing resources. This copies the shared pointers to
    /// internal map resulting in reference counter incrementation, which will prevent deletion
    /// of the resource until tracker decrements the counter.
    /// \param resources Resources accessed by the GPU
    /// \param fenceValue Fence value indicating end of GPU usage
    void registerUsage(const std::set<ID3D12PageablePtr> &resources, uint64_t fenceValue);

    /// Notify the tracker about GPU accessing descriptors. This moves the DescriptorAllocation objects
    /// to internal map. The allocation will be deleted (which will trigger freeing space on heap)
    /// then fence reaches specified value.
    /// \param descriptorAllocations Descriptors accessed by the GPU
    /// \param fenceValue Fence value indicating end of GPU usage
    void registerUsage(std::vector<DescriptorAllocation> &descriptorAllocations, uint64_t fenceValue);

    /// Releases reference counters of all tracked resources which are no longer in use
    /// by the GPU. This does not guarantee deletion, because the shared pointer can
    /// still be referenced somewhere else
    void performDeletion(uint64_t fenceValue);

private:
    void validateResourceFence(ID3D12Pageable *resource, uint64_t fenceValue);
    void validateResourceFence(const DescriptorAllocation &descriptors, uint64_t fenceValue);

    std::map<ID3D12PageablePtr, uint64_t> resourceUsageMap;
    std::map<DescriptorAllocation, uint64_t> descriptorUsageMap;
};

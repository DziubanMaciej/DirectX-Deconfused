#pragma once

#include "DXD/NonCopyableAndMovable.h"
#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <map>
#include <set>

class ResourceUsageTracker : DXD::NonCopyableAndMovable {
public:
    /// Notify the tracker about GPU accessing resources. This copies the shared pointers to
    /// internal map resulting in reference counter incrementation, which will prevent deletion
    /// of the resource until tracker decrements the counter
    /// \param resources Resources accessed by the GPU
    /// \param fenceValue Fence value indicating and of GPU usage
    void registerUsage(const std::set<ID3D12ResourcePtr> &resources, uint64_t fenceValue);

    /// Releases reference counters of all tracked resources which are no longer in use
    /// by the GPU. This does not guarantee deletion, because the shared pointer can
    /// still be referenced somewhere else
    void performDeletion(uint64_t fenceValue);

private:
    void validateResourceFence(ID3D12Resource *resource, uint64_t fenceValue);

    std::map<ID3D12ResourcePtr, uint64_t> resourceUsageMap;
};

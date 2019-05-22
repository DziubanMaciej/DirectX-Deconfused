#include "ResourceUsageTracker.h"

#include <cassert>

void ResourceUsageTracker::registerUsage(const std::set<ID3D12ResourcePtr> &resources, uint64_t fenceValue) {
    for (const ID3D12ResourcePtr &resource : resources) {
#ifdef _DEBUG
        validateResourceFence(resource.Get(), fenceValue);
#endif
        this->resourceUsageMap[resource.Get()] = fenceValue;
    }
}
#include "Utility/GetComPtrRefCount.h"
void ResourceUsageTracker::performDeletion(uint64_t fenceValue) {
    for (auto it = resourceUsageMap.begin(); it != resourceUsageMap.end();) {
        if (it->second <= fenceValue) {
            ID3D12ResourcePtr a = it->first;
            const auto adad = getComPtrRefCount(a);
            it = resourceUsageMap.erase(it);
        } else {
            it++;
        }
    }
}

void ResourceUsageTracker::validateResourceFence(ID3D12Resource *resource, uint64_t fenceValue) {
    auto iterator = this->resourceUsageMap.find(resource);
    assert(iterator == this->resourceUsageMap.end() || iterator->second < fenceValue);
}

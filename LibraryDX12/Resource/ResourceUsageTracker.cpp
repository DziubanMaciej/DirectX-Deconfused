#include "ResourceUsageTracker.h"

#include <cassert>

void ResourceUsageTracker::registerUsage(const std::set<ID3D12PageablePtr> &resources, uint64_t fenceValue) {
    for (const ID3D12PageablePtr &resource : resources) {
#ifdef _DEBUG
        validateResourceFence(resource.Get(), fenceValue);
#endif
        this->resourceUsageMap[resource.Get()] = fenceValue;
    }
}

void ResourceUsageTracker::registerUsage(std::vector<DescriptorAllocation> &descriptorAllocations, uint64_t fenceValue) {
    for (auto allocationIndex = 0u; allocationIndex < descriptorAllocations.size(); allocationIndex++) {
        DescriptorAllocation &allocation = descriptorAllocations[allocationIndex];
#ifdef _DEBUG
        validateResourceFence(allocation, fenceValue);
#endif
        this->descriptorUsageMap[std::move(allocation)] = fenceValue;
    }
}

void ResourceUsageTracker::performDeletion(uint64_t fenceValue) {
    for (auto it = resourceUsageMap.begin(); it != resourceUsageMap.end();) {
        if (it->second <= fenceValue) {
            it = resourceUsageMap.erase(it);
        } else {
            it++;
        }
    }
    for (auto it = descriptorUsageMap.begin(); it != descriptorUsageMap.end();) {
        if (it->second <= fenceValue) {
            it = descriptorUsageMap.erase(it);
        } else {
            it++;
        }
    }
}

void ResourceUsageTracker::validateResourceFence(ID3D12Pageable *resource, uint64_t fenceValue) {
    auto iterator = this->resourceUsageMap.find(resource);
    assert(iterator == this->resourceUsageMap.end() || iterator->second < fenceValue);
}

void ResourceUsageTracker::validateResourceFence(const DescriptorAllocation &descriptors, uint64_t fenceValue) {
    auto iterator = this->descriptorUsageMap.find(descriptors);
    assert(iterator == this->descriptorUsageMap.end() || iterator->second < fenceValue);
}

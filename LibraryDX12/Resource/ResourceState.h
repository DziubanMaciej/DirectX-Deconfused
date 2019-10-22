#pragma once

#include <ExternalHeaders/Wrappers/d3dx12.h>

class Resource;

class ResourceState {
public:
    constexpr static UINT maxSubresourcesCount = 10;
    struct BarriersCreationData {
        BarriersCreationData(Resource &resource) : resource(resource) {}
        Resource &resource;
        D3D12_RESOURCE_BARRIER barriers[maxSubresourcesCount] = {};
        UINT barriersCount = 0u;
    };

    explicit ResourceState(D3D12_RESOURCE_STATES state);
    ResourceState &operator=(D3D12_RESOURCE_STATES state);

    void setState(D3D12_RESOURCE_STATES state, UINT subresource, BarriersCreationData *outBarriers = nullptr);
    D3D12_RESOURCE_STATES getState(UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) const;
    bool areAllSubresourcesInState(D3D12_RESOURCE_STATES state) const;

private:
    D3D12_RESOURCE_STATES resourceState;
    bool hasSubresourceSpecificState = false;
    D3D12_RESOURCE_STATES subresourcesStates[maxSubresourcesCount] = {};
};

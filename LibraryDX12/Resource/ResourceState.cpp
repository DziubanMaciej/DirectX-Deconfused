#include "Resource/Resource.h"
#include "Resource/ResourceState.h"

#include <cassert>

ResourceState::ResourceState(D3D12_RESOURCE_STATES state)
    : resourceState(state) {}

ResourceState &ResourceState::operator=(D3D12_RESOURCE_STATES state) {
    resourceState = state;
    hasSubresourceSpecificState = false;
    return *this;
}

void ResourceState::setState(D3D12_RESOURCE_STATES state, UINT subresource, BarriersCreationData *outBarriers) {
    // Helper lambda function to avoid duplication
    auto addBarrier = [this, outBarriers, state](UINT subresource) {
        const auto currentState = getState(subresource);
        if (currentState != state) {
            auto &barrier = outBarriers->barriers[outBarriers->barriersCount++];
            barrier = CD3DX12_RESOURCE_BARRIER::Transition(outBarriers->resource.getResource().Get(), currentState, state, subresource);
        }
    };

    // Setting all subresources to the same state
    if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
        // Generate transition barriers
        if (outBarriers) {
            if (this->hasSubresourceSpecificState) {
                for (auto currentSubresource = 0u; currentSubresource < outBarriers->resource.getSubresourcesCount(); currentSubresource++) {
                    addBarrier(currentSubresource);
                }
            } else {
                addBarrier(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
            }
        }
        // Update data
        this->resourceState = state;
        this->hasSubresourceSpecificState = false;

    }
    // Setting state for only one subresource
    else {
        // Generate transition barrier
        if (outBarriers) {
            addBarrier(subresource);
        }

        // Set all subresources to correct state
        if (!this->hasSubresourceSpecificState) {
            std::fill_n(this->subresourcesStates, maxSubresourcesCount, this->resourceState);
            this->hasSubresourceSpecificState = true;
        }

        // Update target subresource
        this->subresourcesStates[subresource] = state;
    };
}

D3D12_RESOURCE_STATES ResourceState::getState(UINT subresource) const {
    if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
        assert(!this->hasSubresourceSpecificState);
        return this->resourceState;
    } else {
        if (hasSubresourceSpecificState) {
            return this->subresourcesStates[subresource];
        }
        return this->resourceState;
    }
}

bool ResourceState::areAllSubresourcesInState(D3D12_RESOURCE_STATES state) const {
    return !hasSubresourceSpecificState && (resourceState == state);
}

#include "ResourceStateController.h"

#include "Resource/Resource.h"

// -------------------------------------------------------------------------- Creating

ResourceStateController::ResourceStateController(CommandList &commandList)
    : commandList(commandList) {}

// -------------------------------------------------------------------------- Transition functions

void ResourceStateController::transition(Resource &resource, D3D12_RESOURCE_STATES destinationState, UINT subresource) {
    const auto knownResourceState = knownResourceStates.find(&resource);
    if (knownResourceState == knownResourceStates.end()) {
        transitionNeverSeenResource(resource, destinationState, subresource);
    } else {
        transitionAlreadySeenResource(resource, destinationState, subresource, knownResourceState->second);
    }
}

void ResourceStateController::transitionNeverSeenResource(Resource &resource, D3D12_RESOURCE_STATES destinationState, UINT subresource) {
    // Initialize with unknown state
    ResourceState state{D3D12_RESOURCE_STATE_UNKNOWN};
    preambleResourceStates[&resource] = state;
    knownResourceStates[&resource] = state;

    // Now resource the resource has been seen
    transitionAlreadySeenResource(resource, destinationState, subresource, state);
}

void ResourceStateController::transitionAlreadySeenResource(Resource &resource, D3D12_RESOURCE_STATES destinationState, UINT subresource, ResourceState currentResourceState) {
    // Generate barriers needed to transition to destinationState
    ResourceState::BarriersCreationData barriers{resource};
    currentResourceState.setState(destinationState, subresource, &barriers);

    // Insert barriers
    for (auto barrierIndex = 0u; barrierIndex < barriers.barriersCount; barrierIndex++) {
        D3D12_RESOURCE_BARRIER barrier = barriers.barriers[barrierIndex];
        if (barrier.Transition.StateBefore == D3D12_RESOURCE_STATE_UNKNOWN) {
            preambleResourceStates[&resource].setState(barrier.Transition.StateAfter, barrier.Transition.Subresource, nullptr);
        } else {
            commandList.addCachedResourceBarrier(std::move(barrier));
        }
    }

    // Register known state
    knownResourceStates[&resource] = std::move(currentResourceState);
}

// -------------------------------------------------------------------------- Functions to be called synchronously

std::vector<D3D12_RESOURCE_BARRIER> ResourceStateController::generatePreambleBarriers() const {
    std::vector<D3D12_RESOURCE_BARRIER> preambleBarriers{};

    for (auto &it : preambleResourceStates) {
        const Resource &resource = *it.first;
        const ResourceState &preambleState = it.second;

        // TODO this object should be const, we only care about the barriers here so barrier code could be
        // extracted from setState method
        ResourceState currentState = resource.getState();

        // Get barriers needed to transition from currentState to preambleState
        ResourceState::BarriersCreationData barriers{resource};
        if (preambleState.areAllSubresourcesInSameState()) {
            currentState.setState(preambleState.getState(), D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, &barriers);
        } else {
            for (auto subresource = 0u; subresource < resource.getSubresourcesCount(); subresource++) {
                // If not all subresources has been transitioned, we may have unknown states which should be ignored
                const auto state = preambleState.getState(subresource);
                if (state != D3D12_RESOURCE_STATE_UNKNOWN) {
                    currentState.setState(state, subresource, &barriers);
                }
            }
        }

        // Put barriers in result object
        const auto barriersStart = barriers.barriers;
        const auto barriersEnd = barriers.barriers + barriers.barriersCount;
        preambleBarriers.insert(preambleBarriers.end(), barriersStart, barriersEnd);
    }

    return std::move(preambleBarriers);
}

void ResourceStateController::applyResourceTransitions() const {
    for (auto it : knownResourceStates) {
        Resource &resource = *it.first;
        ResourceState currentState = resource.getState();
        const ResourceState &destinationState = it.second;

        if (destinationState.areAllSubresourcesInSameState()) {
            currentState.setState(destinationState.getState(), D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, nullptr);
        } else {
            for (auto subresource = 0u; subresource < resource.getSubresourcesCount(); subresource++) {
                // If not all subresources has been transitioned, we may have unknown states which should be ignored
                const auto state = destinationState.getState(subresource);
                if (state != D3D12_RESOURCE_STATE_UNKNOWN) {
                    currentState.setState(state, subresource, nullptr);
                }
            }
        }

        resource.setState(currentState);
    }
}

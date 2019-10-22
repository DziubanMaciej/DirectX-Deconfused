#pragma once

#include "Resource/ResourceState.h"

#include <DXD/Utility/NonCopyableAndMovable.h>
#include <unordered_map>

class CommandList;
class Resource;

class ResourceStateController {
public:
    ResourceStateController(CommandList &commandList);

    void transition(Resource &resource, D3D12_RESOURCE_STATES destinationState, UINT subresource);
    std::vector<D3D12_RESOURCE_BARRIER> generatePreambleBarriers() const;
    void applyResourceTransitions() const;

private:
    void transitionNeverSeenResource(Resource &resource, D3D12_RESOURCE_STATES destinationState,
                                     UINT subresource);
    void transitionAlreadySeenResource(Resource &resource, D3D12_RESOURCE_STATES destinationState,
                                       UINT subresource, ResourceState currentResourceState);

    CommandList &commandList;
    std::unordered_map<Resource *, ResourceState> preambleResourceStates = {};
    std::unordered_map<Resource *, ResourceState> knownResourceStates = {};
};

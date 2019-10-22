#include "Application/ApplicationImpl.h"
#include "CommandList/CommandList.h"
#include "Resource/Resource.h"
#include "Resource/ResourceStateController.h"

#include <gtest/gtest.h>

struct ResourceStateControllerTests : ::testing::Test {
    struct CommandQueueMock : CommandQueue {
        using CommandQueue::CommandQueue;
        using CommandQueue::resourceUsageTracker;
    };

    struct ResourceMock : Resource {
        using Resource::Resource;
        using Resource::state;
    };

    void SetUp() {
        this->resource = std::make_unique<ResourceMock>();
        this->resource->setResource(nullptr, D3D12_RESOURCE_STATE_COMMON, 3u);

        this->commandQueue = std::make_unique<CommandQueueMock>(ApplicationImpl::getInstance().getDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);
        this->commandList = std::make_unique<CommandList>(*commandQueue);
        this->resourceStateController = std::make_unique<ResourceStateController>(*commandList);
    }

    void TearDown() {
        // TODO this is needed to satisfy an assert which checks for unsubmitted command lists.
        // Tt can be disabled for ULTs though.
        this->commandList->registerAllData(commandQueue->resourceUsageTracker, 1u);
    }

    D3D12_RESOURCE_BARRIER expectedPreambleBarrier(D3D12_RESOURCE_STATES beforeState,
                                                   D3D12_RESOURCE_STATES afterState, UINT subresource) {
        return CD3DX12_RESOURCE_BARRIER::Transition(resource->getResource().Get(), beforeState,
                                                    afterState, subresource);
    }

    D3D12_RESOURCE_BARRIER expectedPreambleBarrier(D3D12_RESOURCE_STATES afterState, UINT subresource) {
        return expectedPreambleBarrier(D3D12_RESOURCE_STATE_COMMON, afterState, subresource);
    }

    std::unique_ptr<ResourceMock> resource;
    std::unique_ptr<CommandQueueMock> commandQueue;
    std::unique_ptr<CommandList> commandList;
    std::unique_ptr<ResourceStateController> resourceStateController;
};

static bool operator==(const D3D12_RESOURCE_BARRIER &left, const D3D12_RESOURCE_BARRIER &right) {
    return memcmp(&left, &right, sizeof(D3D12_RESOURCE_BARRIER)) == 0;
}

TEST_F(ResourceStateControllerTests, givenSubresourceSpecificTransitionsWhenGettingPreambleBarriersThenTransitionSubresourcesIndividually) {
    // Those should be preamble barriers
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDEX_BUFFER, 0);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 1);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 2);

    // Later barriers
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, 2);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_DEPTH_READ, 0);

    // Verify
    const auto barriers = resourceStateController->generatePreambleBarriers();
    EXPECT_EQ(3u, barriers.size());
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER, 0), barriers[0]);
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 1), barriers[1]);
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 2), barriers[2]);
}

TEST_F(ResourceStateControllerTests, givenWholeResourceTransitionWhenGettingPreambleBarriersThenTransitionWholeResourceInOneBarrier) {
    // This should be preamble barrier
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    // Later barriers
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, 2);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_DEPTH_READ, 0);

    // Verify
    const auto barriers = resourceStateController->generatePreambleBarriers();
    EXPECT_EQ(1u, barriers.size());
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES), barriers[0]);
}

TEST_F(ResourceStateControllerTests, givenSubresourceSpecificTransitionAndWholeResourceTransitionWhenGettingPreambleBarriersThenTransitionAllSubresourcesIndividually) {
    // Those should be preamble barrier
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 1);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDEX_BUFFER, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    // Later barriers
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, 2);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_DEPTH_READ, 0);

    // Verify
    const auto barriers = resourceStateController->generatePreambleBarriers();
    EXPECT_EQ(3u, barriers.size());
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER, 0), barriers[0]);
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 1), barriers[1]);
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_INDEX_BUFFER, 2), barriers[2]);
}

TEST_F(ResourceStateControllerTests, givenNotAllSubresourceSpecificTransitionsWhenGettingPreambleBarriersThenTransitionOnlyReferencedSubresources) {
    // Those should be preamble barrier, subresource0 is not transitioned here
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 1);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 2);

    // Later barriers
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, 2);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_DEPTH_READ, 1);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, 1);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 1);

    // Verify
    const auto barriers = resourceStateController->generatePreambleBarriers();
    EXPECT_EQ(2u, barriers.size());
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 1), barriers[0]);
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 2), barriers[1]);
}

TEST_F(ResourceStateControllerTests, givenSubresourceSpecificStatesAndWholeResourceTransitionWhenGettingPreambleBarriersThenTransitionAllSubresourcesIndividually) {
    // Set resource state
    resource->state.setState(D3D12_RESOURCE_STATE_DEPTH_WRITE, 0);
    resource->state.setState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 1);
    resource->state.setState(D3D12_RESOURCE_STATE_RENDER_TARGET, 2);

    // This should be preamble barrier
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    // Later barriers
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, 2);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_DEPTH_READ, 1);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 0);

    // Verify
    const auto barriers = resourceStateController->generatePreambleBarriers();
    EXPECT_EQ(2u, barriers.size());
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0), barriers[0]);
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 2), barriers[1]);
}

TEST_F(ResourceStateControllerTests, givenSubresourceSpecificStatesAndSubresourceSpecificTransitionsWhenGettingPreambleBarriersThenTransitionAllSubresourcesIndividually) {
    // Set resource state
    resource->state.setState(D3D12_RESOURCE_STATE_DEPTH_WRITE, 0);
    resource->state.setState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 1);
    resource->state.setState(D3D12_RESOURCE_STATE_RENDER_TARGET, 2);

    // Those should be preamble barriers, subresource1 is not transitioned at all
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 2);

    // Later barriers
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, 2);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_DEPTH_READ, 0);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 0);

    // Verify
    const auto barriers = resourceStateController->generatePreambleBarriers();
    EXPECT_EQ(2u, barriers.size());
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0), barriers[0]);
    EXPECT_EQ(expectedPreambleBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 2), barriers[1]);
}

TEST_F(ResourceStateControllerTests, givenWholeResourceTransitionAsLastWhenApplyingTransitionsThenTransitionResourceToProperState) {
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 2);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_DEPTH_READ, 0);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    resourceStateController->generatePreambleBarriers();
    EXPECT_TRUE(resource->state.areAllSubresourcesInState(D3D12_RESOURCE_STATE_COMMON));
    resourceStateController->applyResourceTransitions();
    EXPECT_TRUE(resource->state.areAllSubresourcesInState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
}

TEST_F(ResourceStateControllerTests, givenSubresourceSpecificTransitionsAsLastWhenApplyingTransitionsThenTransitionSubresourcesToProperStates) {
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 2);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_DEPTH_READ, 0);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 1);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_RENDER_TARGET, 2);

    resourceStateController->generatePreambleBarriers();
    EXPECT_TRUE(resource->state.areAllSubresourcesInState(D3D12_RESOURCE_STATE_COMMON));
    resourceStateController->applyResourceTransitions();
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_READ, resource->state.getState(0));
    EXPECT_EQ(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, resource->state.getState(1));
    EXPECT_EQ(D3D12_RESOURCE_STATE_RENDER_TARGET, resource->state.getState(2));
}

TEST_F(ResourceStateControllerTests, givenNotAllSubresourceSpecificTransitionsAsLastWhenApplyingTransitionsThenTransitionReferencedSubresourcesToProperStates) {
    // resource1 is not transitioned
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 0);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, 2);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_DEPTH_READ, 0);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, 2);
    resourceStateController->transition(*resource, D3D12_RESOURCE_STATE_RENDER_TARGET, 2);

    resourceStateController->generatePreambleBarriers();
    EXPECT_TRUE(resource->state.areAllSubresourcesInState(D3D12_RESOURCE_STATE_COMMON));
    resourceStateController->applyResourceTransitions();
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_READ, resource->state.getState(0));
    EXPECT_EQ(D3D12_RESOURCE_STATE_COMMON, resource->state.getState(1));
    EXPECT_EQ(D3D12_RESOURCE_STATE_RENDER_TARGET, resource->state.getState(2));
}

#include "Resource/Resource.h"

#include <DXD/Application.h>
#include <gtest/gtest.h>

TEST(ResourceStateTests, givenDefaultConstructedStateThenItIsInvalid) {
    ResourceState state{};
    EXPECT_TRUE(state.areAllSubresourcesInState(D3D12_RESOURCE_STATE_UNKNOWN));
}

TEST(ResourceStateTests, givenSameStateForAllSubresourcesWhenGettingStateThanReturnCorrectValues) {
    ResourceState state{D3D12_RESOURCE_STATE_DEPTH_WRITE};
    EXPECT_TRUE(state.areAllSubresourcesInState(D3D12_RESOURCE_STATE_DEPTH_WRITE));
    EXPECT_TRUE(state.areAllSubresourcesInSameState());
    EXPECT_FALSE(state.areAllSubresourcesInState(D3D12_RESOURCE_STATE_DEPTH_READ));
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_WRITE, state.getState(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES));
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_WRITE, state.getState(0));
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_WRITE, state.getState(1));
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_WRITE, state.getState(2));
}

TEST(ResourceStateTests, givenDifferentStatesForSubresourcesWhenGettingStateThanReturnCorrectValues) {
    ResourceState state{D3D12_RESOURCE_STATE_DEPTH_WRITE};
    state.setState(D3D12_RESOURCE_STATE_COMMON, 0);
    state.setState(D3D12_RESOURCE_STATE_RENDER_TARGET, 3);

    EXPECT_FALSE(state.areAllSubresourcesInSameState());
    EXPECT_FALSE(state.areAllSubresourcesInState(D3D12_RESOURCE_STATE_DEPTH_WRITE));
    EXPECT_FALSE(state.areAllSubresourcesInState(D3D12_RESOURCE_STATE_COMMON));
    EXPECT_FALSE(state.areAllSubresourcesInState(D3D12_RESOURCE_STATE_RENDER_TARGET));

    EXPECT_EQ(D3D12_RESOURCE_STATE_COMMON, state.getState(0));
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_WRITE, state.getState(1));
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_WRITE, state.getState(2));
    EXPECT_EQ(D3D12_RESOURCE_STATE_RENDER_TARGET, state.getState(3));
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_WRITE, state.getState(4));
}

TEST(ResourceStateTests, givenDifferentStatesForSubresourcesWhenSettingStateForAllThenReturnCorrectValues) {
    ResourceState state{D3D12_RESOURCE_STATE_DEPTH_WRITE};
    state.setState(D3D12_RESOURCE_STATE_COMMON, 2);
    state.setState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    EXPECT_TRUE(state.areAllSubresourcesInSameState());
    EXPECT_TRUE(state.areAllSubresourcesInState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}

struct ResourceStateWithBarriersTests : ::testing::Test {
    void SetUp() override {
        resource = std::make_unique<Resource>();
        resource->setResource(nullptr, D3D12_RESOURCE_STATE_DEPTH_WRITE, 4u);
        barriers = std::make_unique<ResourceState::BarriersCreationData>(*resource);
        state = std::make_unique<ResourceState>(D3D12_RESOURCE_STATE_DEPTH_WRITE);
    }

    std::unique_ptr<Resource> resource;
    std::unique_ptr<ResourceState::BarriersCreationData> barriers;
    std::unique_ptr<ResourceState> state;
};

TEST_F(ResourceStateWithBarriersTests, givenSameStateForSubresourceWhenTransitioningToTheSameStateThenNoBarriersAreMade) {
    state->setState(D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, barriers.get());
    EXPECT_EQ(0u, barriers->barriersCount);
}

TEST_F(ResourceStateWithBarriersTests, givenSameStateForSubresourceWhenTransitioningSingleSubresourceToTheSameStateThenNoBarriersAreMade) {
    state->setState(D3D12_RESOURCE_STATE_DEPTH_WRITE, 3, barriers.get());
    EXPECT_EQ(0u, barriers->barriersCount);
}

TEST_F(ResourceStateWithBarriersTests, givenSameStateForSubresourceWhenTransitioningToOtherStateThenBarrierIsMade) {
    state->setState(D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, barriers.get());
    EXPECT_EQ(1u, barriers->barriersCount);
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_WRITE, barriers->barriers[0].Transition.StateBefore);
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_READ, barriers->barriers[0].Transition.StateAfter);
    EXPECT_EQ(resource->getResource().Get(), barriers->barriers[0].Transition.pResource);
    EXPECT_EQ(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, barriers->barriers[0].Transition.Subresource);
}

TEST_F(ResourceStateWithBarriersTests, givenSameStateForSubresourceWhenTransitioningSingleSubresourceToOtherStateThenBarrierIsMade) {
    state->setState(D3D12_RESOURCE_STATE_DEPTH_READ, 3u, barriers.get());
    EXPECT_EQ(1u, barriers->barriersCount);
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_WRITE, barriers->barriers[0].Transition.StateBefore);
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_READ, barriers->barriers[0].Transition.StateAfter);
    EXPECT_EQ(resource->getResource().Get(), barriers->barriers[0].Transition.pResource);
    EXPECT_EQ(3u, barriers->barriers[0].Transition.Subresource);
}

TEST_F(ResourceStateWithBarriersTests, givenSubresourceSpecificStateWhenTransitioningSubresourceToTheSameStateThenNoBarrierIsMade) {
    state->setState(D3D12_RESOURCE_STATE_DEPTH_READ, 3u, nullptr);
    state->setState(D3D12_RESOURCE_STATE_DEPTH_READ, 3u, barriers.get());
    EXPECT_EQ(0u, barriers->barriersCount);
}

TEST_F(ResourceStateWithBarriersTests, givenSubresourceSpecificStateWhenTransitioningSubresourceToOtherStateThenBarrierIsMade) {
    state->setState(D3D12_RESOURCE_STATE_DEPTH_READ, 3u, nullptr);
    state->setState(D3D12_RESOURCE_STATE_COMMON, 3u, barriers.get());
    EXPECT_EQ(1u, barriers->barriersCount);
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_READ, barriers->barriers[0].Transition.StateBefore);
    EXPECT_EQ(D3D12_RESOURCE_STATE_COMMON, barriers->barriers[0].Transition.StateAfter);
    EXPECT_EQ(resource->getResource().Get(), barriers->barriers[0].Transition.pResource);
    EXPECT_EQ(3u, barriers->barriers[0].Transition.Subresource);
}

TEST_F(ResourceStateWithBarriersTests, givenSubresourceSpecificStatesWhenTransitioningWholeResourceToOtherStateThenOnlyNeededBarriersAreMade) {
    state->setState(D3D12_RESOURCE_STATE_DEPTH_READ, 0u, nullptr);
    state->setState(D3D12_RESOURCE_STATE_COMMON, 3u, nullptr);
    state->setState(D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, barriers.get());

    EXPECT_EQ(2u, barriers->barriersCount);

    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_READ, barriers->barriers[0].Transition.StateBefore);
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_WRITE, barriers->barriers[0].Transition.StateAfter);
    EXPECT_EQ(resource->getResource().Get(), barriers->barriers[0].Transition.pResource);
    EXPECT_EQ(0u, barriers->barriers[0].Transition.Subresource);

    EXPECT_EQ(D3D12_RESOURCE_STATE_COMMON, barriers->barriers[1].Transition.StateBefore);
    EXPECT_EQ(D3D12_RESOURCE_STATE_DEPTH_WRITE, barriers->barriers[1].Transition.StateAfter);
    EXPECT_EQ(resource->getResource().Get(), barriers->barriers[1].Transition.pResource);
    EXPECT_EQ(3u, barriers->barriers[1].Transition.Subresource);
}

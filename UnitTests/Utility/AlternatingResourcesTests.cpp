#include "Resource/Resource.h"
#include "Utility/AlternatingResources.h"

#include <gtest/gtest.h>

struct MockAlternatingResources : AlternatingResources {
    MockAlternatingResources() : AlternatingResources(L"", ID3D12DevicePtr{}) {}
    using AlternatingResources::resources;
    bool resizeCalled = false;
    void resize(int width, int height) override {
        resources[0] = std::make_unique<Resource>();
        resources[1] = std::make_unique<Resource>();
        resizeCalled = true;
    }
};

TEST(AlternatingResourcesTests, givenAlternatingResourcesCreatedThenItIsEmpty) {
    MockAlternatingResources resources{};
    EXPECT_FALSE(resources.resizeCalled);
    EXPECT_EQ(nullptr, resources.resources[0]);
    EXPECT_EQ(nullptr, resources.resources[1]);
}

TEST(AlternatingResourcesTests, givenResizedAlternatingResourcesWhenCallingGettersThenItReturnsCorrectResources) {
    MockAlternatingResources resources{};
    EXPECT_EQ(resources.resources[0].get(), &resources.getSource());
    EXPECT_EQ(resources.resources[1].get(), &resources.getDestination());

    resources.swapResources();
    EXPECT_EQ(resources.resources[1].get(), &resources.getSource());
    EXPECT_EQ(resources.resources[0].get(), &resources.getDestination());

    resources.swapResources();
    EXPECT_EQ(resources.resources[0].get(), &resources.getSource());
    EXPECT_EQ(resources.resources[1].get(), &resources.getDestination());
}

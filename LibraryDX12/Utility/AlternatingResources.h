#pragma once

#include <DXD/ExternalHeadersWrappers/d3d12.h>

/// Encapsulates two resources - source and destination resource - which can be easily
/// swapped, so the source becomes destination and vice versa.
struct AlternatingResources {
    explicit AlternatingResources(ID3D12DevicePtr &device) : device(device) {}
    virtual void resize(int width, int height) = 0;

    Resource &getSource() { return *resources[sourceResourceIndex]; }
    Resource &getDestination() { return *resources[1 - sourceResourceIndex]; }
    Resource &getDestination(bool last, Resource &finalDestination) { return last ? finalDestination : getDestination(); }

    void swapResources() {
        sourceResourceIndex = 1 - sourceResourceIndex;
    }

protected:
    ID3D12DevicePtr device = {};
    int sourceResourceIndex = 0;
    std::unique_ptr<Resource> resources[2] = {};
};

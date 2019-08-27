#pragma once 

#include "DXD/ExternalHeadersWrappers/d3d12.h"

/// Encapsulates two resources - source and destination resource - which can be easily
/// swapped, so the source becomes destination and vice versa.
struct AlternatingResources {
    explicit AlternatingResources(ID3D12DevicePtr &device) : device(device) {}
    virtual void resize(int width, int height) = 0;

    RenderTarget &getSource() { return *resources[sourceResourceIndex]; }
    RenderTarget &getDestination() { return *resources[1 - sourceResourceIndex]; }
    RenderTarget &getDestination(bool last, RenderTarget &finalDestination) { return last ? finalDestination : getDestination(); }

    void swapResources() {
        sourceResourceIndex = 1 - sourceResourceIndex;
    }

protected:
    ID3D12DevicePtr device = {};
    int sourceResourceIndex = 0;
    std::unique_ptr<RenderTarget> resources[2] = {};
};
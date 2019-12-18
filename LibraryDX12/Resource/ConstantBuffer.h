#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "Resource/Resource.h"
#include "Utility/MathHelper.h"

#include <memory>

class DescriptorController;

// TODO: mapped buffer volatile?
/// Encapsulates a linear buffer created on upload heap meant to be used as a constant
/// buffer view. Internally it manages subbufers in its allocation to avoid data races.
/// User should call getData() to get pointer to the staging allocation, fill it with
/// desired data and call uploadAndSwap() to copy staged memory to the GPU buffer.
class ConstantBuffer : public Resource {
public:
    ConstantBuffer(UINT size, UINT subbuffersCount);
    ~ConstantBuffer() override;

    /// Returns staging allocation for uploading data to constant buffer. Intermediate
    /// memory is needed to avoid doing many access to buffer memory, but rather upload
    /// it all at once
    /// \return pointer to staging allocation
    template <typename CbType>
    CbType *getData() {
        this->dirtyStagingData = true;
        uint8_t *address = stagingData.get() + getSubbufferOffset(subbufferSize, currentSubbufferIndex);
        return reinterpret_cast<CbType *>(address);
    }

    /// Upload staging data to the GPU memory and swap to the next subbuffer.
    /// \return descriptor of just uploaded subbuffer
    D3D12_CPU_DESCRIPTOR_HANDLE uploadAndSwap();

private:
    // Helpers
    static uint8_t *map(ID3D12ResourcePtr &resource);
    static UINT64 getAlignedSubbufferSize(UINT64 subbufferSize);
    static UINT64 getTotalBufferSize(UINT64 subbufferSize, UINT subbuffersCount);
    static UINT64 getSubbufferOffset(UINT64 subbufferSize, UINT subbufferIndex);
    void createDescriptors();

    // Constants
    const UINT64 subbufferSize;
    const UINT64 totalSize;
    const UINT subbuffersCount;

    // Allocations
    uint8_t *const mappedConstantBuffer;
    std::unique_ptr<uint8_t[]> stagingData;
    DescriptorAllocation descriptors;

    // Changing data
    UINT currentSubbufferIndex = 0u;
    bool dirtyStagingData = false;
};

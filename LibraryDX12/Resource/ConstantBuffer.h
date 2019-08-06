#pragma once

#include "Descriptor/CpuDescriptorAllocation.h"
#include "Resource/Resource.h"
#include "Utility/BitHelper.h"

#include <memory>

class DescriptorManager;

// TODO: mapped buffer volatile?
// TODO: buffer versioning
class ConstantBuffer : public Resource {
public:
    ConstantBuffer(ID3D12DevicePtr device, DescriptorManager &descriptorManager, UINT size);
    ~ConstantBuffer() override;

    template <typename CbType>
    CbType *getData() { return reinterpret_cast<CbType *>(data.get()); }
    uint8_t *getRawData() { return data.get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE getCbvHandle() const { return descriptor.getCpuHandle(); }

    void upload();

private:
    static void *map(ID3D12ResourcePtr &resource);
    void createDescriptor(ID3D12DevicePtr &device);

    const CpuDescriptorAllocation descriptor;
    const UINT size;
    void *const mappedConstantBuffer;
    std::unique_ptr<uint8_t[]> data;
};

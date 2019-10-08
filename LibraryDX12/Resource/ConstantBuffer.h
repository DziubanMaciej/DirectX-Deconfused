#pragma once

#include "Descriptor/DescriptorAllocation.h"
#include "Resource/Resource.h"
#include "Utility/MathHelper.h"

#include <memory>

class DescriptorController;

// TODO: mapped buffer volatile?
// TODO: buffer versioning
class ConstantBuffer : public Resource {
public:
    ConstantBuffer(ID3D12DevicePtr device, DescriptorController &descriptorController, UINT size);
    ~ConstantBuffer() override;

    template <typename CbType>
    CbType *getData() { return reinterpret_cast<CbType *>(data.get()); }
    uint8_t *getRawData() { return data.get(); }

    void upload();

private:
    static void *map(ID3D12ResourcePtr &resource);
    void createDescriptor(ID3D12DevicePtr &device);

    const UINT size;
    void *const mappedConstantBuffer;
    std::unique_ptr<uint8_t[]> data;
};

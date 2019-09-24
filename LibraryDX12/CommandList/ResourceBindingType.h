#pragma once

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include <functional>

namespace ResourceBindingType {
enum ResourceBindingType {
    Unknown,
    Graphics,
    Compute,
    COUNT
};

using SetRootDescriptorTableFunction = std::function<void(ID3D12GraphicsCommandList *, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)>;
using SetRootSignatureFunction = std::function<void(ID3D12GraphicsCommandList *, ID3D12RootSignature *)>;
using SetRoot32BitConstantsFunction = std::function<void(ID3D12GraphicsCommandList *, UINT, UINT, const void *, UINT)>;

extern SetRootDescriptorTableFunction setRootDescriptorTableFunctions[COUNT];
extern SetRootSignatureFunction setRootSignatureFunctions[COUNT];
extern SetRoot32BitConstantsFunction setRoot32BitConstantsFunctions[COUNT];
} // namespace ResourceBindingType

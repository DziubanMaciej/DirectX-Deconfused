#include "ResourceBindingType.h"

namespace ResourceBindingType {
SetRootDescriptorTableFunction setRootDescriptorTableFunctions[] = {
    nullptr,
    &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable,
    &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable};

SetRootSignatureFunction setRootSignatureFunctions[] = {
    nullptr,
    &ID3D12GraphicsCommandList::SetGraphicsRootSignature,
    &ID3D12GraphicsCommandList::SetComputeRootSignature};

} // namespace ResourceBindingType

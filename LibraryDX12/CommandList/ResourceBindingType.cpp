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

SetRoot32BitConstantsFunction setRoot32BitConstantsFunctions[] = {
    nullptr,
    &ID3D12GraphicsCommandList::SetGraphicsRoot32BitConstants,
    &ID3D12GraphicsCommandList::SetComputeRoot32BitConstants};

} // namespace ResourceBindingType

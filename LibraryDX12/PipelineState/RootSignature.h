#pragma once

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <map>
#include <vector>

class RootSignature : DXD::NonCopyableAndMovable {
public:
    template <typename ConstantType>
    RootSignature &append32bitConstant(D3D12_SHADER_VISIBILITY visibility);
    RootSignature &appendShaderResourceView(D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility);
    RootSignature &appendConstantBufferView(D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility);
    RootSignature &appendUnorderedAccessView(D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility);
    RootSignature &appendStaticSampler(const D3D12_STATIC_SAMPLER_DESC &samplerDescription);
    RootSignature &appendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility);

    ID3D12RootSignaturePtr compile(ID3D12DevicePtr device);

private:
    UINT getNextShaderRegisterAndIncrement(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors);
    void resolveDescriptorRanges();
    static D3D_ROOT_SIGNATURE_VERSION getHighestRootSignatureVersion(ID3D12DevicePtr device);

    std::map<D3D12_SHADER_VISIBILITY, std::vector<D3D12_DESCRIPTOR_RANGE1>> descriptorRanges; // intermediate data, has to be resolved to descriptor tables

    std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters = {};        // goes directly to D3D12_VERSIONED_ROOT_SIGNATURE_DESC
    std::vector<D3D12_STATIC_SAMPLER_DESC> samplerDescriptions = {}; // goes directly to D3D12_VERSIONED_ROOT_SIGNATURE_DESC

    UINT nextShaderRegisterT = 0; // SRV
    UINT nextShaderRegisterU = 0; // UAV
    UINT nextShaderRegisterB = 0; // CBV, including 32bit constants
    UINT nextShaderRegisterS = 0; // Sampler
};

template <typename ConstantType>
inline RootSignature &RootSignature::append32bitConstant(D3D12_SHADER_VISIBILITY visibility) {
    static_assert(sizeof(ConstantType) % 4 == 0, "Not a dword aligned type");
    constexpr static auto dwordCount = sizeof(ConstantType) / 4;
    rootParameters.emplace_back();
    rootParameters.back().InitAsConstants(dwordCount, nextShaderRegisterB++, 0, visibility);
    return *this;
}

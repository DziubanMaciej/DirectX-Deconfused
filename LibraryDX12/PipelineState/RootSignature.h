#pragma once

#include "PipelineState/DescriptorTable.h"
#include "PipelineState/ShaderRegister.h"

#include "DXD/NonCopyableAndMovable.h"

#include <ExternalHeaders/Wrappers/d3dx12.h>
#include <map>
#include <vector>

class StaticSampler;

class RootSignature : DXD::NonCopyableAndMovable {
public:
    template <typename ConstantType>
    RootSignature &append32bitConstant(CbvShaderRegister reg, D3D12_SHADER_VISIBILITY visibility);
    RootSignature &appendShaderResourceView(SrvShaderRegister reg, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility);
    RootSignature &appendConstantBufferView(CbvShaderRegister reg, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility);
    RootSignature &appendUnorderedAccessView(UavShaderRegister reg, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility);
    RootSignature &appendStaticSampler(SamplerShaderRegister reg, const StaticSampler &staticSampler);
    RootSignature &appendDescriptorTable(DescriptorTable &&table);

    RootSignature &compile(ID3D12DevicePtr device);

    auto getRootSignature() { return rootSignature; }
    const auto &getRootParameters() const { return rootParameters; }

private:
    static D3D_ROOT_SIGNATURE_VERSION getHighestRootSignatureVersion(ID3D12DevicePtr device);

    // Storage for allocations
    std::vector<DescriptorTable> descriptorTables = {};

    // Arguments which go directly to D3D12_VERSIONED_ROOT_SIGNATURE_DESC
    std::vector<D3D12_ROOT_PARAMETER1> rootParameters = {};
    std::vector<D3D12_STATIC_SAMPLER_DESC> samplerDescriptions = {};

    // Result
    ID3D12RootSignaturePtr rootSignature;
};

template <typename ConstantType>
inline RootSignature &RootSignature::append32bitConstant(CbvShaderRegister reg, D3D12_SHADER_VISIBILITY visibility) {
    static_assert(sizeof(ConstantType) % 4 == 0, "Not a dword aligned type");
    constexpr static auto dwordCount = sizeof(ConstantType) / 4;
    CD3DX12_ROOT_PARAMETER1 parameter = {};
    parameter.InitAsConstants(dwordCount, reg, 0, visibility);
    rootParameters.push_back(std::move(parameter));
    return *this;
}

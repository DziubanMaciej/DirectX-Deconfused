#include "RootSignature.h"

#include "PipelineState/StaticSampler.h"
#include "Utility/ThrowIfFailed.h"

#include <cassert>

RootSignature &RootSignature::appendShaderResourceView(SrvShaderRegister reg, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility) {
    CD3DX12_ROOT_PARAMETER1 parameter = {};
    parameter.InitAsShaderResourceView(reg, 0, flags, visibility);
    rootParameters.push_back(std::move(parameter));
    return *this;
}

RootSignature &RootSignature::appendConstantBufferView(CbvShaderRegister reg, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility) {
    CD3DX12_ROOT_PARAMETER1 parameter = {};
    parameter.InitAsConstantBufferView(reg, 0, flags, visibility);
    rootParameters.push_back(std::move(parameter));
    return *this;
}

RootSignature &RootSignature::appendUnorderedAccessView(UavShaderRegister reg, D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility) {
    CD3DX12_ROOT_PARAMETER1 parameter = {};
    parameter.InitAsUnorderedAccessView(reg, 0, flags, visibility);
    rootParameters.push_back(std::move(parameter));
    return *this;
}

RootSignature &RootSignature::appendStaticSampler(SamplerShaderRegister reg, const StaticSampler &staticSampler) {
    samplerDescriptions.push_back(staticSampler.get());
    return *this;
}

RootSignature &RootSignature::appendDescriptorTable(DescriptorTable &&table) {
    rootParameters.push_back(table.getTable());
    descriptorTables.push_back(std::move(table));
    return *this;
}

D3D_ROOT_SIGNATURE_VERSION RootSignature::getHighestRootSignatureVersion(ID3D12DevicePtr device) {
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))) {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
    return featureData.HighestVersion;
}

RootSignature &RootSignature::compile(ID3D12DevicePtr device) {
    const D3D_ROOT_SIGNATURE_VERSION highestVersion = getHighestRootSignatureVersion(device);
    const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription = {
        static_cast<UINT>(rootParameters.size()), rootParameters.data(),
        static_cast<UINT>(samplerDescriptions.size()), samplerDescriptions.data(),
        rootSignatureFlags};

    ID3DBlobPtr rootSignatureBlob = {};
    ID3DBlobPtr errorBlob = {};
    throwIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription, highestVersion, &rootSignatureBlob, &errorBlob), errorBlob.Get());

    ID3D12RootSignaturePtr rootSignature = {};
    throwIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&this->rootSignature)));
    return *this;
}

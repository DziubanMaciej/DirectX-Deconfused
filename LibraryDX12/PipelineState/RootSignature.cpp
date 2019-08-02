#include "RootSignature.h"

#include "Utility/ThrowIfFailed.h"

#include <cassert>

void RootSignature::resolveDescriptorRanges() {
    for (auto it = descriptorRanges.begin(); it != descriptorRanges.end(); it++) {
        const D3D12_SHADER_VISIBILITY visibility = it->first;
        const std::vector<D3D12_DESCRIPTOR_RANGE1> &rangesForGivenVisibility = it->second;

        assert(descriptorRanges.size() > 0);

        rootParameters.emplace_back();
        rootParameters.back().InitAsDescriptorTable(static_cast<UINT>(rangesForGivenVisibility.size()), rangesForGivenVisibility.data(), visibility);
    }
}

RootSignature &RootSignature::appendShaderResourceView(D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility) {
    rootParameters.emplace_back();
    rootParameters.back().InitAsShaderResourceView(nextShaderRegisterT++, 0, flags, visibility);
    return *this;
}

RootSignature &RootSignature::appendConstantBufferView(D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility) {
    rootParameters.emplace_back();
    rootParameters.back().InitAsConstantBufferView(nextShaderRegisterB++, 0, flags, visibility);
    return *this;
}

RootSignature &RootSignature::appendUnorderedAccessView(D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility) {
    rootParameters.emplace_back();
    rootParameters.back().InitAsUnorderedAccessView(nextShaderRegisterU++, 0, flags, visibility);
    return *this;
}

RootSignature &RootSignature::appendStaticSampler(const D3D12_STATIC_SAMPLER_DESC &samplerDescription) {
    samplerDescriptions.push_back(samplerDescription);
    return *this;
}

UINT RootSignature::getNextShaderRegisterAndIncrement(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors) {
    UINT result{};
    switch (rangeType) {
    case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
        result = nextShaderRegisterT;
        nextShaderRegisterT += numDescriptors;
        break;
    case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
        result = nextShaderRegisterU;
        nextShaderRegisterU += numDescriptors;
        break;
    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
        result = nextShaderRegisterB;
        nextShaderRegisterB += numDescriptors;
        break;
    case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
        result = nextShaderRegisterS;
        nextShaderRegisterS += numDescriptors;
        UNREACHABLE_CODE(); // TODO Sampler descriptors cannot be in one table with SRVs,UAVs and CBVs. resolveDescriptorRanges() divide them
    default:
        UNREACHABLE_CODE();
    }
    return result;
}

RootSignature &RootSignature::appendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors, D3D12_SHADER_VISIBILITY visibility) {
    const auto baseShaderRegister = getNextShaderRegisterAndIncrement(rangeType, numDescriptors);
    D3D12_DESCRIPTOR_RANGE1 range{rangeType, numDescriptors, baseShaderRegister, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND};
    this->descriptorRanges[visibility].push_back(std::move(range));
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
    resolveDescriptorRanges();
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

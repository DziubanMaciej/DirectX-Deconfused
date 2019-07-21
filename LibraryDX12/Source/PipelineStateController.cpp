#include "PipelineStateController.h"

#include "Source/ConstantBuffers.h"
#include "Utility/ThrowIfFailed.h"

#include "DXD/ExternalHeadersWrappers/DirectXMath.h"
#include <cassert>

D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineStateController::getBaseGraphicsPipelineSateDesc() {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
    pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pipelineStateDesc.SampleMask = UINT_MAX;
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;
    pipelineStateDesc.SampleDesc.Count = 1;
    return pipelineStateDesc;
}

PipelineStateController::PipelineStateController(ID3D12DevicePtr device) : device(device) {}

void PipelineStateController::compileAll() {
    for (int identifier = 0; identifier < static_cast<int>(Identifier::COUNT); identifier++) {
        getPipelineState(static_cast<Identifier>(identifier));
    }
}

ID3D12PipelineStatePtr PipelineStateController::getPipelineState(Identifier identifier) {
    compile(identifier);
    return pipelineStates[static_cast<int>(identifier)];
}

ID3D12RootSignaturePtr PipelineStateController::getRootSignature(Identifier identifier) {
    compile(identifier);
    return rootSignatures[static_cast<int>(identifier)];
}

void PipelineStateController::compile(Identifier identifier) {
    if (pipelineStates[static_cast<int>(identifier)] != nullptr) {
        return;
    }

    ID3D12RootSignaturePtr rootSignature = {};
    ID3D12PipelineStatePtr pipelineState = {};

    switch (identifier) {
    case Identifier::PIPELINE_STATE_DEFAULT:
        compilePipelineStateDefault(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_TEXTURE:
        compilePipelineStateTexture(rootSignature, pipelineState);
        break;
    default:
        unreachableCode();
    }

    pipelineStates[static_cast<int>(identifier)] = std::move(pipelineState);
    rootSignatures[static_cast<int>(identifier)] = std::move(rootSignature);
}

void PipelineStateController::compilePipelineStateDefault(ID3D12RootSignaturePtr &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    rootSignature = RootSignature{}
                        .append32bitConstant<SimpleConstantBuffer>(D3D12_SHADER_VISIBILITY_VERTEX)
                        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Shaders
    ID3DBlobPtr vertexShaderBlob = loadAndCompileShader(L"vertex.hlsl", vertexShaderTarget);
    ID3DBlobPtr pixelShaderBlob = loadAndCompileShader(L"pixel.hlsl", pixelShaderTarget);

    // Compilation
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = getBaseGraphicsPipelineSateDesc();
    desc.pRootSignature = rootSignature.Get();
    desc.VS = D3D12_SHADER_BYTECODE{vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize()};
    desc.PS = D3D12_SHADER_BYTECODE{pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize()};
    desc.InputLayout = D3D12_INPUT_LAYOUT_DESC{inputLayout, _countof(inputLayout)};
    throwIfFailed(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState)));
}

void PipelineStateController::compilePipelineStateTexture(ID3D12RootSignaturePtr &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    CD3DX12_STATIC_SAMPLER_DESC linearClampSampler(0, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    rootSignature = RootSignature{}
                        .append32bitConstant<SimpleConstantBuffer>(D3D12_SHADER_VISIBILITY_VERTEX)
                        .appendStaticSampler(linearClampSampler)
                        .appendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1)
                        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Shaders
    ID3DBlobPtr vertexShaderBlob = loadAndCompileShader(L"vertex.hlsl", vertexShaderTarget);
    ID3DBlobPtr pixelShaderBlob = loadAndCompileShader(L"pixel.hlsl", pixelShaderTarget);

    // Compilation
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = getBaseGraphicsPipelineSateDesc();
    desc.pRootSignature = rootSignature.Get();
    desc.VS = D3D12_SHADER_BYTECODE{vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize()};
    desc.PS = D3D12_SHADER_BYTECODE{pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize()};
    desc.InputLayout = D3D12_INPUT_LAYOUT_DESC{inputLayout, _countof(inputLayout)};
    throwIfFailed(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pipelineState)));
}

ID3DBlobPtr PipelineStateController::loadBlob(const std::wstring &path) {
    ID3DBlobPtr blob = {};
    throwIfFailed(D3DReadFileToBlob(path.c_str(), &blob));
    return blob;
}

ID3DBlobPtr PipelineStateController::loadAndCompileShader(const std::wstring &name, const std::string &target) {
    ID3DBlob *compiledShader = nullptr;
    ID3DBlob *errorBlob = nullptr;

    const auto path = std::wstring{SHADERS_PATH} + name;
    const auto result = D3DCompileFromFile(path.c_str(), nullptr, nullptr, "main", target.c_str(), 0, 0, &compiledShader, &errorBlob);
    throwIfFailed(result, errorBlob);
    return ID3DBlobPtr{compiledShader};
}

// ----------------------------------------------------------------------------------------------- RootSignature helper

void PipelineStateController::RootSignature::resolveDescriptorRanges() {
    // TODO We create one descriptor table with D3D12_SHADER_VISIBILITY_ALL, while they could be more specialized
    if (descriptorRanges.size() > 0) {
        rootParameters.emplace_back();
        rootParameters.back().InitAsDescriptorTable(static_cast<UINT>(descriptorRanges.size()), descriptorRanges.data());
    }
}

PipelineStateController::RootSignature &PipelineStateController::RootSignature::appendShaderResourceView(D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility) {
    rootParameters.emplace_back();
    rootParameters.back().InitAsShaderResourceView(nextShaderRegisterT++, 0, flags, visibility);
    return *this;
}

PipelineStateController::RootSignature &PipelineStateController::RootSignature::appendConstantBufferView(D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility) {
    rootParameters.emplace_back();
    rootParameters.back().InitAsConstantBufferView(nextShaderRegisterB++, 0, flags, visibility);
    return *this;
}

PipelineStateController::RootSignature &PipelineStateController::RootSignature::appendUnorderedAccessView(D3D12_ROOT_DESCRIPTOR_FLAGS flags, D3D12_SHADER_VISIBILITY visibility) {
    rootParameters.emplace_back();
    rootParameters.back().InitAsUnorderedAccessView(nextShaderRegisterU++, 0, flags, visibility);
    return *this;
}

PipelineStateController::RootSignature &PipelineStateController::RootSignature::appendStaticSampler(const D3D12_STATIC_SAMPLER_DESC &samplerDescription) {
    samplerDescriptions.push_back(samplerDescription);
    return *this;
}

UINT PipelineStateController::RootSignature::getNextShaderRegisterAndIncrement(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors) {
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
        break;
    default:
        unreachableCode();
    }
    return result;
}

PipelineStateController::RootSignature &PipelineStateController::RootSignature::appendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors) {
    const auto baseShaderRegister = getNextShaderRegisterAndIncrement(rangeType, numDescriptors);
    D3D12_DESCRIPTOR_RANGE1 range{rangeType, numDescriptors, baseShaderRegister, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND};
    descriptorRanges.push_back(std::move(range));
    return *this;
}

D3D_ROOT_SIGNATURE_VERSION PipelineStateController::RootSignature::getHighestRootSignatureVersion(ID3D12DevicePtr device) {
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))) {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }
    return featureData.HighestVersion;
}

ID3D12RootSignaturePtr PipelineStateController::RootSignature::compile(ID3D12DevicePtr device) {
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
    throwIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
    return rootSignature;
}

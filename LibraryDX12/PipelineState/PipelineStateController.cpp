#include "PipelineStateController.h"

#include "PipelineState/RootSignature.h"
#include "Resource/ConstantBuffers.h"
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
    case Identifier::PIPELINE_STATE_NORMAL:
        compilePipelineStateNormal(rootSignature, pipelineState);
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
                        .append32bitConstant<ModelMvp>(D3D12_SHADER_VISIBILITY_VERTEX)                            // register(b0)
                        .appendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, D3D12_SHADER_VISIBILITY_PIXEL) // register(b1)
                        .append32bitConstant<ObjectProperties>(D3D12_SHADER_VISIBILITY_PIXEL)                     // register(b2)
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

void PipelineStateController::compilePipelineStateNormal(ID3D12RootSignaturePtr &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data

    rootSignature = RootSignature{}
                        .append32bitConstant<ModelMvp>(D3D12_SHADER_VISIBILITY_VERTEX)                            // register(b0)
                        .appendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, D3D12_SHADER_VISIBILITY_PIXEL) // register(b1)
                        .append32bitConstant<ObjectProperties>(D3D12_SHADER_VISIBILITY_PIXEL)                     // register(b2)
                        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Shaders
    ID3DBlobPtr vertexShaderBlob = loadAndCompileShader(L"vertex_normal.hlsl", vertexShaderTarget);
    ID3DBlobPtr pixelShaderBlob = loadAndCompileShader(L"pixel_normal.hlsl", pixelShaderTarget);

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
                        .append32bitConstant<ModelMvp>(D3D12_SHADER_VISIBILITY_VERTEX)                            // register(b0)
                        .appendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, D3D12_SHADER_VISIBILITY_PIXEL) // register(b1)
                        .append32bitConstant<ObjectProperties>(D3D12_SHADER_VISIBILITY_PIXEL)                     // register(b2)
                        .appendStaticSampler(linearClampSampler)                                                  // register(s0)
                        //.appendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, D3D12_SHADER_VISIBILITY_PIXEL)
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

ID3DBlobPtr PipelineStateController::loadAndCompileShader(const std::wstring &name, const std::string &target) {
    ID3DBlob *compiledShader = nullptr;
    ID3DBlob *errorBlob = nullptr;

    const auto path = std::wstring{SHADERS_PATH} + name;
    const auto result = D3DCompileFromFile(path.c_str(), nullptr, nullptr, "main", target.c_str(), 0, 0, &compiledShader, &errorBlob);
    throwIfFailed(result, errorBlob);
    return ID3DBlobPtr{compiledShader};
}
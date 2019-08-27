#include "PipelineStateController.h"

#include "PipelineState/DescriptorTable.h"
#include "PipelineState/PipelineState.h"
#include "PipelineState/RootSignature.h"
#include "PipelineState/ShaderRegister.h"
#include "Resource/ConstantBuffers.h"
#include "Utility/ThrowIfFailed.h"

#include "DXD/ExternalHeadersWrappers/DirectXMath.h"
#include <cassert>

// --------------------------------------------------------------------------------------------- General methods

using namespace ShaderRegisterHelpers;

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

RootSignature &PipelineStateController::getRootSignature(Identifier identifier) {
    compile(identifier);
    return rootSignatures[static_cast<int>(identifier)];
}

void PipelineStateController::compile(Identifier identifier) {
    const auto index = static_cast<int>(identifier);
    if (pipelineStates[index] != nullptr) {
        return;
    }

    RootSignature &rootSignature = this->rootSignatures[index];
    ID3D12PipelineStatePtr &pipelineState = this->pipelineStates[index];

    switch (identifier) {
    case Identifier::PIPELINE_STATE_UNKNOWN:
        break;
    case Identifier::PIPELINE_STATE_DEFAULT:
        compilePipelineStateDefault(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_TEXTURE_NORMAL:
        compilePipelineStateTextureNormal(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_NORMAL:
        compilePipelineStateNormal(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_POST_PROCESS_BLACK_BARS:
        compilePipelineStatePostProcessBlackBars(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_SM_NORMAL:
        compilePipelineStateShadowMapNormal(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_SM_TEXTURE_NORMAL:
        compilePipelineStateShadowMapTextureNormal(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_POST_PROCESS_CONVOLUTION:
        compilePipelineStatePostProcessConvolution(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_POST_PROCESS_LINEAR_COLOR_CORRECTION:
        compilePipelineStatePostProcessLinearColorCorrection(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR:
        compilePipelineStatePostProcessGaussianBlur(rootSignature, pipelineState);
        break;
    default:
        UNREACHABLE_CODE();
    }
}

// --------------------------------------------------------------------------------------------- 3D

void PipelineStateController::compilePipelineStateDefault(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    DescriptorTable table{D3D12_SHADER_VISIBILITY_PIXEL};
    table.appendCbvRange(b(1), 1);
    rootSignature
        .append32bitConstant<ModelMvp>(b(0), D3D12_SHADER_VISIBILITY_VERTEX)
        .append32bitConstant<ObjectProperties>(b(2), D3D12_SHADER_VISIBILITY_PIXEL)
        .appendDescriptorTable(std::move(table))
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Pipeline state object
    PipelineState{inputLayout, rootSignature}
        .VS(L"vertex.hlsl")
        .PS(L"pixel.hlsl")
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStateNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    DescriptorTable table{D3D12_SHADER_VISIBILITY_PIXEL};
    table.appendCbvRange(b(1), 1);
    table.appendSrvRange(t(0), 8); // shadow maps
    rootSignature
        .append32bitConstant<ModelMvp>(b(0), D3D12_SHADER_VISIBILITY_VERTEX)
        .append32bitConstant<ObjectProperties>(b(2), D3D12_SHADER_VISIBILITY_PIXEL)
        .appendDescriptorTable(std::move(table))
        .appendStaticSampler(s(0), sampler)
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Pipeline state object
    PipelineState{inputLayout, rootSignature}
        .VS(L"vertex_normal.hlsl")
        .PS(L"pixel_normal.hlsl")
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStateTextureNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    DescriptorTable table{D3D12_SHADER_VISIBILITY_PIXEL};
    table.appendCbvRange(b(1), 1);
    table.appendSrvRange(t(0), 1); // diffuse texture
    table.appendSrvRange(t(1), 8); // shadow maps
    rootSignature
        .append32bitConstant<ModelMvp>(b(0), D3D12_SHADER_VISIBILITY_VERTEX)
        .append32bitConstant<ObjectProperties>(b(2), D3D12_SHADER_VISIBILITY_PIXEL)
        .appendDescriptorTable(std::move(table))
        .appendStaticSampler(s(0), sampler)
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Pipeline state object
    PipelineState{inputLayout, rootSignature}
        .VS(L"vertex_normal_texture.hlsl")
        .PS(L"pixel_normal_texture.hlsl")
        .compile(device, pipelineState);
}

// --------------------------------------------------------------------------------------------- Shadow maps

void PipelineStateController::compilePipelineStateShadowMapNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    rootSignature
        .append32bitConstant<SMmvp>(b(0), D3D12_SHADER_VISIBILITY_VERTEX) // register(b0)
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Pipeline state object
    PipelineState{inputLayout, rootSignature}
        .VS(L"vertex_normal_sm.hlsl")
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStateShadowMapTextureNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    rootSignature
        .append32bitConstant<SMmvp>(b(0), D3D12_SHADER_VISIBILITY_VERTEX) // register(b0)
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Pipeline state object
    PipelineState{inputLayout, rootSignature}
        .VS(L"vertex_normal_texture_sm.hlsl")
        .compile(device, pipelineState);
}

// --------------------------------------------------------------------------------------------- Post processes

template <typename ConstantType>
static void compileBasicPipelineStatePostProcess(ID3D12DevicePtr device, RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState, const std::wstring &psPath) {
    // Root signature - crossthread data
    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    DescriptorTable table{D3D12_SHADER_VISIBILITY_PIXEL};
    table.appendSrvRange(t(0), 1);

    rootSignature
        .appendStaticSampler(s(0), sampler)
        .append32bitConstant<ConstantType>(b(0), D3D12_SHADER_VISIBILITY_PIXEL)
        .appendDescriptorTable(std::move(table))
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Pipeline state object
    PipelineState{inputLayout, rootSignature}
        .VS(L"vertex_post_process.hlsl")
        .PS(psPath)
        .disableDepthStencil()
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStatePostProcessBlackBars(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    compileBasicPipelineStatePostProcess<PostProcessBlackBarsCB>(device, rootSignature, pipelineState, L"pixel_post_process_black_bars.hlsl");
}

void PipelineStateController::compilePipelineStatePostProcessConvolution(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    compileBasicPipelineStatePostProcess<PostProcessConvolutionCB>(device, rootSignature, pipelineState, L"pixel_post_process_convolution.hlsl");
}

void PipelineStateController::compilePipelineStatePostProcessLinearColorCorrection(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    compileBasicPipelineStatePostProcess<PostProcessLinearColorCorrectionCB>(device, rootSignature, pipelineState, L"pixel_post_process_linear_color_correction.hlsl");
}

void PipelineStateController::compilePipelineStatePostProcessGaussianBlur(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    compileBasicPipelineStatePostProcess<PostProcessGaussianBlurCB>(device, rootSignature, pipelineState, L"pixel_post_process_gaussian_blur.hlsl");
}

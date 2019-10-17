#include "PipelineStateController.h"

#include "PipelineState/DescriptorTable.h"
#include "PipelineState/PipelineState.h"
#include "PipelineState/RootSignature.h"
#include "PipelineState/ShaderRegister.h"
#include "PipelineState/StaticSampler.h"
#include "Resource/ConstantBuffers.h"
#include "Utility/ThrowIfFailed.h"

#include <DXD/ExternalHeadersWrappers/DirectXMath.h>
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
    case Identifier::PIPELINE_STATE_NORMAL:
        compilePipelineStateNormal(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_TEXTURE_NORMAL:
        compilePipelineStateTextureNormal(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_TEXTURE_NORMAL_MAP:
        compilePipelineStateTextureNormalMap(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_GENERATE_MIPS:
        compilePipelineStateGenerateMips(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_SSAO:
        compilePipelineStateSSAO(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_SSR:
        compilePipelineStateSSR(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_LIGHTING:
        compilePipelineStateLighting(rootSignature, pipelineState);
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
    case Identifier::PIPELINE_STATE_SM_TEXTURE_NORMAL_MAP:
        compilePipelineStateShadowMapTextureNormalMap(rootSignature, pipelineState);
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
    case Identifier::PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR_COMPUTE_HORIZONTAL:
        compilePipelineStatePostProcessGaussianBlurComputeHorizontal(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR_COMPUTE_VERTICAL:
        compilePipelineStatePostProcessGaussianBlurComputeVertical(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_POST_PROCESS_APPLY_BLOOM:
        compilePipelineStatePostProcessApplyBloom(rootSignature, pipelineState);
        break;
    case Identifier::PIPELINE_STATE_SPRITE:
        compilePipelineStateSprite(rootSignature, pipelineState);
        break;
    default:
        UNREACHABLE_CODE();
    }
}

// --------------------------------------------------------------------------------------------- 3D

void PipelineStateController::compilePipelineStateNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    rootSignature
        .append32bitConstant<ModelMvp>(b(0), D3D12_SHADER_VISIBILITY_VERTEX)
        .append32bitConstant<ObjectProperties>(b(2), D3D12_SHADER_VISIBILITY_PIXEL)
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Pipeline state object
    GraphicsPipelineState{inputLayout, rootSignature}
        .VS(L"3D/normal_VS.hlsl")
        .PS(L"3D/normal_PS.hlsl")
        .setRenderTargetsCount(3)
        .setRenderTargetFormat(1, DXGI_FORMAT_R16G16B16A16_SNORM)
        .setRenderTargetFormat(2, DXGI_FORMAT_R8G8_UNORM)
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStateTextureNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    StaticSampler sampler{D3D12_SHADER_VISIBILITY_PIXEL};
    sampler.addressMode(D3D12_TEXTURE_ADDRESS_MODE_MIRROR);
    sampler.filter(D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR);
    DescriptorTable table{D3D12_SHADER_VISIBILITY_PIXEL};
    table.appendSrvRange(t(0), 1); // diffuse texture
    rootSignature
        .append32bitConstant<NormalTextureCB>(b(0), D3D12_SHADER_VISIBILITY_VERTEX)
        .append32bitConstant<ObjectProperties>(b(1), D3D12_SHADER_VISIBILITY_PIXEL)
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
    GraphicsPipelineState{inputLayout, rootSignature}
        .VS(L"3D/normal_texture_VS.hlsl")
        .PS(L"3D/normal_texture_PS.hlsl")
        .setRenderTargetsCount(3)
        .setRenderTargetFormat(1, DXGI_FORMAT_R16G16B16A16_SNORM)
        .setRenderTargetFormat(2, DXGI_FORMAT_R8G8_UNORM)
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStateTextureNormalMap(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    StaticSampler sampler{D3D12_SHADER_VISIBILITY_PIXEL};
    sampler.addressMode(D3D12_TEXTURE_ADDRESS_MODE_MIRROR);
    DescriptorTable table{D3D12_SHADER_VISIBILITY_PIXEL};
    table.appendSrvRange(t(0), 1); // normal map
    table.appendSrvRange(t(1), 1); // diffuse texture
    rootSignature
        .append32bitConstant<NormalTextureCB>(b(0), D3D12_SHADER_VISIBILITY_VERTEX)
        .append32bitConstant<ObjectProperties>(b(1), D3D12_SHADER_VISIBILITY_PIXEL)
        .appendDescriptorTable(std::move(table))
        .appendStaticSampler(s(0), sampler)
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Pipeline state object
    GraphicsPipelineState{inputLayout, rootSignature}
        .VS(L"3D/texture_normal_map_VS.hlsl")
        .PS(L"3D/texture_normal_map_PS.hlsl")
        .setRenderTargetsCount(3)
        .setRenderTargetFormat(1, DXGI_FORMAT_R16G16B16A16_SNORM)
        .setRenderTargetFormat(2, DXGI_FORMAT_R8G8_UNORM)
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
    GraphicsPipelineState{inputLayout, rootSignature}
        .setDsvFormat(DXGI_FORMAT_D16_UNORM)
        .VS(L"ShadowMap/normal_VS.hlsl")
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
    GraphicsPipelineState{inputLayout, rootSignature}
        .setDsvFormat(DXGI_FORMAT_D16_UNORM)
        .VS(L"ShadowMap/normal_texture_VS.hlsl")
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStateShadowMapTextureNormalMap(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    rootSignature
        .append32bitConstant<SMmvp>(b(0), D3D12_SHADER_VISIBILITY_VERTEX) // register(b0)
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Pipeline state object
    GraphicsPipelineState{inputLayout, rootSignature}
        .setDsvFormat(DXGI_FORMAT_D16_UNORM)
        .VS(L"ShadowMap/texture_normal_map_VS.hlsl")
        .compile(device, pipelineState);
}

// --------------------------------------------------------------------------------------------- Mip maps

void PipelineStateController::compilePipelineStateGenerateMips(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    StaticSampler sampler{D3D12_SHADER_VISIBILITY_ALL};
    sampler.filter(D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT)
        .addressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    DescriptorTable table{D3D12_SHADER_VISIBILITY_ALL};
    table.appendSrvRange(t(0), 1)
        .appendUavRange(u(0), 1);

    // Root signature - crossthread data
    rootSignature
        .appendDescriptorTable(std::move(table))
        .append32bitConstant<GenerateMipsCB>(b(0), D3D12_SHADER_VISIBILITY_ALL)
        .appendStaticSampler(s(0), sampler)
        .compile(device);

    // Compile shaders
    return ComputePipelineState{rootSignature}
        .CS(L"generate_mip_maps_CS.hlsl", nullptr)
        .compile(device, pipelineState);
}

// --------------------------------------------------------------------------------------------- Post processes

template <typename ConstantType>
static void compileBasicPipelineStatePostProcess(ID3D12DevicePtr device, RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState, const std::wstring &psPath) {
    // Root signature - crossthread data
    StaticSampler sampler{D3D12_SHADER_VISIBILITY_PIXEL};
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
    return GraphicsPipelineState{inputLayout, rootSignature}
        .VS(L"PostProcess/VS.hlsl")
        .PS(psPath)
        .disableDepthStencil()
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStatePostProcessBlackBars(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    compileBasicPipelineStatePostProcess<PostProcessBlackBarsCB>(device, rootSignature, pipelineState, L"PostProcess/black_bars_PS.hlsl");
}

void PipelineStateController::compilePipelineStatePostProcessConvolution(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    compileBasicPipelineStatePostProcess<PostProcessConvolutionCB>(device, rootSignature, pipelineState, L"PostProcess/convolution_PS.hlsl");
}

void PipelineStateController::compilePipelineStatePostProcessLinearColorCorrection(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    compileBasicPipelineStatePostProcess<PostProcessLinearColorCorrectionCB>(device, rootSignature, pipelineState, L"PostProcess/linear_color_correction_PS.hlsl");
}

void PipelineStateController::compilePipelineStatePostProcessGaussianBlur(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    compileBasicPipelineStatePostProcess<PostProcessGaussianBlurCB>(device, rootSignature, pipelineState, L"PostProcess/gaussian_blur_PS.hlsl");
}

inline void compilePipelineStatePostProcessGaussianBlurCompute(ID3D12DevicePtr &device, RootSignature &rootSignature,
                                                               ID3D12PipelineStatePtr &pipelineState, const D3D_SHADER_MACRO *shaderDefines) {
    StaticSampler sampler{D3D12_SHADER_VISIBILITY_ALL};
    DescriptorTable table{D3D12_SHADER_VISIBILITY_ALL};
    table.appendSrvRange(t(0), 1)
        .appendUavRange(u(0), 1);

    // Root signature - crossthread data
    rootSignature
        .appendDescriptorTable(std::move(table))
        .append32bitConstant<PostProcessGaussianBlurComputeCB>(b(0), D3D12_SHADER_VISIBILITY_ALL)
        .appendStaticSampler(s(0), sampler)
        .compile(device);

    // Compile shaders
    return ComputePipelineState{rootSignature}
        .CS(L"PostProcess/gaussian_blur_CS.hlsl", shaderDefines)
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStatePostProcessGaussianBlurComputeHorizontal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    const D3D_SHADER_MACRO shaderDefines[] = {
        {"HORIZONTAL", ""},
        {nullptr, nullptr}};
    compilePipelineStatePostProcessGaussianBlurCompute(device, rootSignature, pipelineState, shaderDefines);
}

void PipelineStateController::compilePipelineStatePostProcessGaussianBlurComputeVertical(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    const D3D_SHADER_MACRO shaderDefines[] = {
        {"VERTICAL", ""},
        {nullptr, nullptr}};
    compilePipelineStatePostProcessGaussianBlurCompute(device, rootSignature, pipelineState, shaderDefines);
}

void PipelineStateController::compilePipelineStatePostProcessApplyBloom(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    StaticSampler sampler{D3D12_SHADER_VISIBILITY_PIXEL};
    DescriptorTable table{D3D12_SHADER_VISIBILITY_PIXEL};
    table.appendSrvRange(t(0), 1);

    rootSignature
        .appendStaticSampler(s(0), sampler)
        .append32bitConstant<PostProcessApplyBloomCB>(b(0), D3D12_SHADER_VISIBILITY_PIXEL)
        .appendDescriptorTable(std::move(table))
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Blend state
    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // Pipeline state object
    return GraphicsPipelineState{inputLayout, rootSignature}
        .VS(L"PostProcess/VS.hlsl")
        .PS(L"PostProcess/apply_bloom_PS.hlsl")
        .disableDepthStencil()
        .setBlendDesc(blendDesc)
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStateSprite(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    StaticSampler sampler{D3D12_SHADER_VISIBILITY_PIXEL};
    DescriptorTable table{D3D12_SHADER_VISIBILITY_PIXEL};
    table.appendSrvRange(t(0), 1);
    rootSignature
        .appendStaticSampler(s(0), sampler)
        .append32bitConstant<SpriteCB>(b(0), D3D12_SHADER_VISIBILITY_PIXEL)
        .appendDescriptorTable(std::move(table))
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    // Blend state
    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // Pipeline state object
    return GraphicsPipelineState{inputLayout, rootSignature}
        .VS(L"PostProcess/VS.hlsl")
        .PS(L"sprite_sampler_PS.hlsl")
        //.PS(L"sampler_PS.hlsl")
        .disableDepthStencil()
        .setBlendDesc(blendDesc)
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStateLighting(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    StaticSampler sampler{D3D12_SHADER_VISIBILITY_PIXEL};
    sampler.filter(D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);

    DescriptorTable table{D3D12_SHADER_VISIBILITY_PIXEL};
    table.appendCbvRange(b(0), 1) // light constant buffer
        .appendSrvRange(t(0), 1)  // gbuffer albedo
        .appendSrvRange(t(1), 1)  // gbuffer normal
        .appendSrvRange(t(2), 1)  // gbuffer specular
        .appendSrvRange(t(3), 1)  // gbuffer depth
        .appendSrvRange(t(4), 1)  // SSAO
        .appendSrvRange(t(5), 8); // shadow maps

    rootSignature
        .appendStaticSampler(s(0), sampler)
        .appendDescriptorTable(std::move(table))
        .append32bitConstant<InverseViewProj>(b(1), D3D12_SHADER_VISIBILITY_PIXEL)
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Pipeline state object
    return GraphicsPipelineState{inputLayout, rootSignature}
        .VS(L"PostProcess/VS.hlsl")
        .PS(L"lighting_PS.hlsl")
        .disableDepthStencil()
        .setRenderTargetsCount(2)
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStateSSAO(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    StaticSampler sampler{D3D12_SHADER_VISIBILITY_PIXEL};
    sampler.lodRange(0, 0);

    DescriptorTable table{D3D12_SHADER_VISIBILITY_PIXEL};
    table.appendSrvRange(t(0), 1) // gbuffer normal
        .appendSrvRange(t(1), 1); // gbuffer depth

    rootSignature
        .appendStaticSampler(s(0), sampler)
        .appendDescriptorTable(std::move(table))
        .append32bitConstant<SsaoCB>(b(1), D3D12_SHADER_VISIBILITY_PIXEL)
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Pipeline state object
    return GraphicsPipelineState{inputLayout, rootSignature}
        .VS(L"PostProcess/VS.hlsl")
        .PS(L"ssao_PS.hlsl")
        .disableDepthStencil()
        .setRenderTargetsCount(1)
        .setRenderTargetFormat(0, DXGI_FORMAT_R8_UNORM)
        .compile(device, pipelineState);
}

void PipelineStateController::compilePipelineStateSSR(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState) {
    // Root signature - crossthread data
    StaticSampler sampler{D3D12_SHADER_VISIBILITY_PIXEL};
    sampler.lodRange(0, 0);

    DescriptorTable table{D3D12_SHADER_VISIBILITY_PIXEL};
    table.appendSrvRange(t(0), 1) // gbuffer normal
        .appendSrvRange(t(1), 1)  // gbuffer depth
        .appendSrvRange(t(2), 1)  // gbuffer specular
        .appendSrvRange(t(3), 1); // lighting output

    rootSignature
        .appendStaticSampler(s(0), sampler)
        .appendDescriptorTable(std::move(table))
        .append32bitConstant<SsrCB>(b(1), D3D12_SHADER_VISIBILITY_PIXEL)
        .compile(device);

    // Input layout - per vertex data
    const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    // Pipeline state object
    return GraphicsPipelineState{inputLayout, rootSignature}
        .VS(L"PostProcess/VS.hlsl")
        .PS(L"ssr_PS.hlsl")
        .disableDepthStencil()
        .setRenderTargetsCount(1)
        .compile(device, pipelineState);
}

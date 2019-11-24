#pragma once

#include "PipelineState/RootSignature.h"

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/d3dcompiler.h>
#include <ExternalHeaders/Wrappers/d3dx12.h>
#include <string>

class PipelineStateController : DXD::NonCopyableAndMovable {
public:
    enum class Identifier {
        PIPELINE_STATE_UNKNOWN,
        // 3D
        PIPELINE_STATE_NORMAL,
        PIPELINE_STATE_TEXTURE_NORMAL,
        PIPELINE_STATE_TEXTURE_NORMAL_MAP,
        // Shadow maps
        PIPELINE_STATE_SM_NORMAL,
        PIPELINE_STATE_SM_TEXTURE_NORMAL,
        PIPELINE_STATE_SM_TEXTURE_NORMAL_MAP,
        // Mip maps
        PIPELINE_STATE_GENERATE_MIPS,
        // SSAO
        PIPELINE_STATE_SSAO,
        // SSR
        PIPELINE_STATE_SSR,
        // SSR
        PIPELINE_STATE_SSR_BLUR,
        // SSR
        PIPELINE_STATE_SSR_MERGE,
        // Lighting
        PIPELINE_STATE_LIGHTING,
        // Fog
        PIPELINE_STATE_FOG,
        // Depth of Field
        PIPELINE_STATE_DOF,
        // Depth of Field
        PIPELINE_STATE_DOF_BLUR,
        // Post Processes
        PIPELINE_STATE_POST_PROCESS_BLACK_BARS,
        PIPELINE_STATE_POST_PROCESS_CONVOLUTION,
        PIPELINE_STATE_POST_PROCESS_LINEAR_COLOR_CORRECTION,
        PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR,
        PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR_COMPUTE_HORIZONTAL,
        PIPELINE_STATE_POST_PROCESS_GAUSSIAN_BLUR_COMPUTE_VERTICAL,
        PIPELINE_STATE_POST_PROCESS_APPLY_BLOOM,
        // Sprite
        PIPELINE_STATE_SPRITE,
        // This hould be the last entry
        COUNT
    };

    PipelineStateController(ID3D12DevicePtr device);

    void compileAll();
    void recompile(Identifier identifier);
    ID3D12PipelineStatePtr getPipelineState(Identifier identifier);
    RootSignature &getRootSignature(Identifier identifier);

private:
    void compile(Identifier identifier);
    void reset(Identifier identifier);

    // 3D
    void compilePipelineStateNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateTextureNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateTextureNormalMap(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // Mip maps
    void compilePipelineStateGenerateMips(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // SSAO
    void compilePipelineStateSSAO(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // SSR
    void compilePipelineStateSSR(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // SSR Merge
    void compilePipelineStateSSRBlur(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // SSR Merge
    void compilePipelineStateSSRMerge(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // Lighting
    void compilePipelineStateLighting(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // Fog
    void compilePipelineStateFog(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // Fog
    void compilePipelineStateDof(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // Fog
    void compilePipelineStateDofBlur(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // Shadow maps
    void compilePipelineStateShadowMapNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateShadowMapTextureNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateShadowMapTextureNormalMap(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // Post processes
    void compilePipelineStatePostProcessBlackBars(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStatePostProcessConvolution(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStatePostProcessLinearColorCorrection(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStatePostProcessGaussianBlur(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStatePostProcessGaussianBlurComputeHorizontal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStatePostProcessGaussianBlurComputeVertical(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStatePostProcessApplyBloom(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateSprite(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);

    ID3D12DevicePtr device;
    ID3D12PipelineStatePtr pipelineStates[static_cast<int>(Identifier::COUNT)] = {};
    RootSignature rootSignatures[static_cast<int>(Identifier::COUNT)] = {};
};

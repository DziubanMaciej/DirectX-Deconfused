#pragma once

#include "PipelineState/RootSignature.h"

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3dcompiler.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <string>

class PipelineStateController : DXD::NonCopyableAndMovable {
public:
    enum class Identifier {
        PIPELINE_STATE_UNKNOWN,
        // 3D
        PIPELINE_STATE_DEFAULT,
        PIPELINE_STATE_TEXTURE_NORMAL,
        PIPELINE_STATE_NORMAL,
        // Shadow maps
        PIPELINE_STATE_SM_NORMAL,
        PIPELINE_STATE_SM_TEXTURE_NORMAL,
        // Post Processes
        PIPELINE_STATE_POST_PROCESS_BLACK_BARS,
        PIPELINE_STATE_POST_PROCESS_CONVOLUTION,
        PIPELINE_STATE_POST_PROCESS_LINEAR_COLOR_CORRECTION,
        // This hould be the last entry
        COUNT
    };

    PipelineStateController(ID3D12DevicePtr device);

    void compileAll();
    ID3D12PipelineStatePtr getPipelineState(Identifier identifier);
    RootSignature &getRootSignature(Identifier identifier);

private:
    void compile(Identifier identifier);

    // 3D
    void compilePipelineStateDefault(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateTextureNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // Shadow maps
    void compilePipelineStateShadowMapNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateShadowMapTextureNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    // Post processes
    void compilePipelineStatePostProcessBlackBars(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStatePostProcessConvolution(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStatePostProcessLinearColorCorrection(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);

    ID3D12DevicePtr device;
    ID3D12PipelineStatePtr pipelineStates[static_cast<int>(Identifier::COUNT)] = {};
    RootSignature rootSignatures[static_cast<int>(Identifier::COUNT)] = {};
};

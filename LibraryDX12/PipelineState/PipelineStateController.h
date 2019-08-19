#pragma once

#include "PipelineState/RootSignature.h"

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3dcompiler.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <string>

class PipelineStateController : DXD::NonCopyableAndMovable {
public:
    enum class Identifier {
        PIPELINE_STATE_UNKNOWN = 0,
        PIPELINE_STATE_DEFAULT = 1,
        PIPELINE_STATE_TEXTURE_NORMAL = 2,
        PIPELINE_STATE_NORMAL = 3,
        PIPELINE_STATE_POST_PROCESS = 4,
        PIPELINE_STATE_SM_NORMAL = 5,
        PIPELINE_STATE_SM_TEXTURE_NORMAL = 6,
        PIPELINE_STATE_POST_PROCESS_CONVOLUTION = 7,
        COUNT // this should be the last entry
    };

    PipelineStateController(ID3D12DevicePtr device);

    void compileAll();
    ID3D12PipelineStatePtr getPipelineState(Identifier identifier);
    RootSignature &getRootSignature(Identifier identifier);

private:
    void compile(Identifier identifier);
    void compilePipelineStateDefault(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateTextureNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStatePostProcess(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateShadowMapNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateShadowMapTextureNormal(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStatePostProcessConvolution(RootSignature &rootSignature, ID3D12PipelineStatePtr &pipelineState);

    ID3D12DevicePtr device;
    ID3D12PipelineStatePtr pipelineStates[static_cast<int>(Identifier::COUNT)] = {};
    RootSignature rootSignatures[static_cast<int>(Identifier::COUNT)] = {};
};

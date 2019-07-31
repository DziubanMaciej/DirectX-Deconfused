#pragma once

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3dcompiler.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <string>

class PipelineStateController : DXD::NonCopyableAndMovable {
public:
    enum class Identifier {
        PIPELINE_STATE_DEFAULT = 0,
        PIPELINE_STATE_TEXTURE = 1,
        PIPELINE_STATE_NORMAL = 2,
        COUNT // this should be the last entry
    };

    PipelineStateController(ID3D12DevicePtr device);

    void compileAll();
    ID3D12PipelineStatePtr getPipelineState(Identifier identifier);
    ID3D12RootSignaturePtr getRootSignature(Identifier identifier);

private:
    void compile(Identifier identifier);
    void compilePipelineStateDefault(ID3D12RootSignaturePtr &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateNormal(ID3D12RootSignaturePtr &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateTexture(ID3D12RootSignaturePtr &rootSignature, ID3D12PipelineStatePtr &pipelineState);

    static D3D12_GRAPHICS_PIPELINE_STATE_DESC getBaseGraphicsPipelineSateDesc();
    static ID3DBlobPtr loadAndCompileShader(const std::wstring &name, const std::string &target);

    ID3D12DevicePtr device;
    ID3D12PipelineStatePtr pipelineStates[static_cast<int>(Identifier::COUNT)] = {};
    ID3D12RootSignaturePtr rootSignatures[static_cast<int>(Identifier::COUNT)] = {};

    constexpr static char pixelShaderTarget[] = "ps_5_1";
    constexpr static char vertexShaderTarget[] = "vs_5_1";
};

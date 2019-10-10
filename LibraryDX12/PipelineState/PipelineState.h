#pragma once

#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/d3d12.h>
#include <DXD/ExternalHeadersWrappers/d3dcompiler.h>
#include <string>
#include <vector>

class RootSignature;

class PipelineState : DXD::NonCopyableAndMovable {
protected:
    D3D12_SHADER_BYTECODE loadAndCompileShader(const std::wstring &name, const std::string &target, const D3D_SHADER_MACRO *defines);
    std::vector<ID3DBlobPtr> shaderBlobs = {};
};

class GraphicsPipelineState : public PipelineState {
public:
    template <typename UINT inputLayoutSize>
    GraphicsPipelineState(const D3D12_INPUT_ELEMENT_DESC (&inputLayout)[inputLayoutSize], RootSignature &rootSignature) {
        description.InputLayout = D3D12_INPUT_LAYOUT_DESC{inputLayout, inputLayoutSize};
        description.pRootSignature = rootSignature.getRootSignature().Get();
        description.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        description.SampleMask = UINT_MAX;
        description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        description.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        description.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        description.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        description.NumRenderTargets = 1;
        description.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        description.SampleDesc.Count = 1;
    }

    GraphicsPipelineState &VS(const std::wstring &path);
    GraphicsPipelineState &PS(const std::wstring &path);
    GraphicsPipelineState &DS(const std::wstring &path);
    GraphicsPipelineState &HS(const std::wstring &path);
    GraphicsPipelineState &GS(const std::wstring &path);
    GraphicsPipelineState &disableDepthStencil();
    GraphicsPipelineState &setRenderTargetsCount(UINT count);
    GraphicsPipelineState &setRenderTargetFormat(UINT idx, DXGI_FORMAT format);
    GraphicsPipelineState &setBlendDesc(const D3D12_BLEND_DESC &blendDesc);
    GraphicsPipelineState &setDsvFormat(const DXGI_FORMAT format);

    void compile(ID3D12DevicePtr device, ID3D12PipelineStatePtr &pipelineState);

private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC description = {};
};

class ComputePipelineState : public PipelineState {
public:
    ComputePipelineState(RootSignature &rootSignature);
    ComputePipelineState &CS(const std::wstring &path, const D3D_SHADER_MACRO *defines);
    void compile(ID3D12DevicePtr device, ID3D12PipelineStatePtr &pipelineState);

private:
    D3D12_COMPUTE_PIPELINE_STATE_DESC description = {};
};

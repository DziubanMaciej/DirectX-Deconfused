#pragma once

#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/d3dcompiler.h"
#include "DXD/ExternalHeadersWrappers/d3dx12.h"
#include <cassert>
#include <string>
#include <vector>

class PipelineStateController : DXD::NonCopyableAndMovable {
public:
    class RootSignature;
    enum class Identifier {
        PIPELINE_STATE_DEFAULT = 0,
        PIPELINE_STATE_TEXTURE = 1,
        COUNT // this should be the last entry
    };

    PipelineStateController(ID3D12DevicePtr device);

    void compileAll();
    ID3D12PipelineStatePtr getPipelineState(Identifier identifier);
    ID3D12RootSignaturePtr getRootSignature(Identifier identifier);

private:
    void compile(Identifier identifier);
    void compilePipelineStateDefault(ID3D12RootSignaturePtr &rootSignature, ID3D12PipelineStatePtr &pipelineState);
    void compilePipelineStateTexture(ID3D12RootSignaturePtr &rootSignature, ID3D12PipelineStatePtr &pipelineState);

    static D3D12_GRAPHICS_PIPELINE_STATE_DESC getBaseGraphicsPipelineSateDesc();
    static ID3DBlobPtr loadBlob(const std::wstring &path);
    static ID3DBlobPtr loadAndCompileShader(const std::wstring &name, const std::string &target);

    ID3D12DevicePtr device;
    ID3D12PipelineStatePtr pipelineStates[static_cast<int>(Identifier::COUNT)] = {};
    ID3D12RootSignaturePtr rootSignatures[static_cast<int>(Identifier::COUNT)] = {};

    constexpr static char pixelShaderTarget[] = "ps_5_1";
    constexpr static char vertexShaderTarget[] = "vs_5_1";
};

class PipelineStateController::RootSignature : DXD::NonCopyableAndMovable {
public:
    template <typename ConstantType>
    RootSignature &append32bitConstant(D3D12_SHADER_VISIBILITY visibility) {
        static_assert(sizeof(ConstantType) % 4 == 0, "Not a dword aligned type");
        constexpr static auto dwordCount = sizeof(ConstantType) / 4;
        rootParameters.emplace_back();
        rootParameters.back().InitAsConstants(dwordCount, nextShaderRegisterB++, 0, visibility);
        return *this;
    }

    RootSignature &appendStaticSampler(const D3D12_STATIC_SAMPLER_DESC &samplerDescription) {
        samplerDescriptions.push_back(samplerDescription);
        return *this;
    }

    RootSignature &appendDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors);

    ID3D12RootSignaturePtr compile(ID3D12DevicePtr device);

    // TODO cbv, srv, uav, descriptor table interfaces

private:
    void resolveDescriptorRanges();
    static D3D_ROOT_SIGNATURE_VERSION getHighestRootSignatureVersion(ID3D12DevicePtr device);

    std::vector<D3D12_DESCRIPTOR_RANGE1> descriptorRanges; // immediate data, has to be resolved to descriptor tables

    std::vector<CD3DX12_ROOT_PARAMETER1> rootParameters = {};        // goes directly to D3D12_VERSIONED_ROOT_SIGNATURE_DESC
    std::vector<D3D12_STATIC_SAMPLER_DESC> samplerDescriptions = {}; // goes directly to D3D12_VERSIONED_ROOT_SIGNATURE_DESC

    UINT nextShaderRegisterT = 0; // SRV
    UINT nextShaderRegisterU = 0; // UAV
    UINT nextShaderRegisterB = 0; // CBV, including 32bit constants
    UINT nextShaderRegisterS = 0; // Sampler
};

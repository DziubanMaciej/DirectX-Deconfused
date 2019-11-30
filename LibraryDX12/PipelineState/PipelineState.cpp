#include "Application/ApplicationImpl.h"
#include "PipelineState/PipelineState.h"
#include "PipelineState/RootSignature.h"
#include "Utility/ThrowIfFailed.h"

GraphicsPipelineState &GraphicsPipelineState::VS(const std::wstring &path) {
    description.VS = loadAndCompileShader(path, "vs_5_1", nullptr);
    return *this;
}

GraphicsPipelineState &GraphicsPipelineState::PS(const std::wstring &path) {
    description.PS = loadAndCompileShader(path, "ps_5_1", nullptr);
    return *this;
}

GraphicsPipelineState &GraphicsPipelineState::DS(const std::wstring &path) {
    description.DS = loadAndCompileShader(path, "ds_5_1", nullptr);
    return *this;
}

GraphicsPipelineState &GraphicsPipelineState::HS(const std::wstring &path) {
    description.HS = loadAndCompileShader(path, "hs_5_1", nullptr);
    return *this;
}

GraphicsPipelineState &GraphicsPipelineState::GS(const std::wstring &path) {
    description.GS = loadAndCompileShader(path, "gs_5_1", nullptr);
    return *this;
}

GraphicsPipelineState &GraphicsPipelineState::disableDepthStencil() {
    description.DepthStencilState.DepthEnable = false;
    description.DepthStencilState.StencilEnable = false;
    return *this;
}

GraphicsPipelineState &GraphicsPipelineState::setRenderTargetsCount(UINT count) {
    description.NumRenderTargets = count;
    std::fill_n(description.RTVFormats, count, DXGI_FORMAT_R8G8B8A8_UNORM);
    return *this;
}

GraphicsPipelineState &GraphicsPipelineState::setRenderTargetFormat(UINT idx, DXGI_FORMAT format) {
    description.RTVFormats[idx] = format;
    return *this;
}

GraphicsPipelineState &GraphicsPipelineState::setBlendDesc(const D3D12_BLEND_DESC &blendDesc) {
    description.BlendState = blendDesc;
    return *this;
}

GraphicsPipelineState &GraphicsPipelineState::setDsvFormat(DXGI_FORMAT format) {
    description.DSVFormat = format;
    return *this;
}

void GraphicsPipelineState::compile(ID3D12DevicePtr device, ID3D12PipelineStatePtr &pipelineState) {
    throwIfFailed(device->CreateGraphicsPipelineState(&description, IID_PPV_ARGS(&pipelineState)));
}

D3D12_SHADER_BYTECODE PipelineState::loadAndCompileShader(const std::wstring &name, const std::string &target, const D3D_SHADER_MACRO *defines) {
    ID3DBlob *compiledShader = nullptr;
    ID3DBlob *errorBlob = nullptr;

    const auto path = std::wstring{SHADERS_PATH} + name;
    const auto includeHandler = D3D_COMPILE_STANDARD_FILE_INCLUDE;

    const UINT compileFlags = ApplicationImpl::getInstance().areDebugShadersEnabled() ? D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION : 0u;
    const auto result = D3DCompileFromFile(path.c_str(), defines, includeHandler, "main", target.c_str(), compileFlags, 0, &compiledShader, &errorBlob);
    throwIfFailed(result, errorBlob);

    this->shaderBlobs.emplace_back(compiledShader);
    return D3D12_SHADER_BYTECODE{compiledShader->GetBufferPointer(), compiledShader->GetBufferSize()};
}

ComputePipelineState::ComputePipelineState(RootSignature &rootSignature) {
    description.pRootSignature = rootSignature.getRootSignature().Get();
}

ComputePipelineState &ComputePipelineState::CS(const std::wstring &path, const D3D_SHADER_MACRO *defines) {
    description.CS = loadAndCompileShader(path, "cs_5_1", defines);
    return *this;
}

void ComputePipelineState::compile(ID3D12DevicePtr device, ID3D12PipelineStatePtr &pipelineState) {
    throwIfFailed(device->CreateComputePipelineState(&description, IID_PPV_ARGS(&pipelineState)));
}

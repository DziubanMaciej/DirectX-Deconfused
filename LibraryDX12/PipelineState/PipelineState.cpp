#include "PipelineState/PipelineState.h"
#include "Utility/ThrowIfFailed.h"

PipelineState &PipelineState::VS(const std::wstring &path) {
    description.VS = loadAndCompileShader(path, "vs_5_1");
    return *this;
}

PipelineState &PipelineState::PS(const std::wstring &path) {
    description.PS = loadAndCompileShader(path, "ps_5_1");
    return *this;
}

PipelineState &PipelineState::DS(const std::wstring &path) {
    description.DS = loadAndCompileShader(path, "ds_5_1");
    return *this;
}

PipelineState &PipelineState::HS(const std::wstring &path) {
    description.HS = loadAndCompileShader(path, "hs_5_1");
    return *this;
}

PipelineState &PipelineState::GS(const std::wstring &path) {
    description.GS = loadAndCompileShader(path, "gs_5_1");
    return *this;
}

PipelineState &PipelineState::disableDepthStencil() {
    description.DepthStencilState.DepthEnable = false;
    description.DepthStencilState.StencilEnable = false;
    return *this;
}

void PipelineState::compile(ID3D12DevicePtr device, ID3D12PipelineStatePtr &pipelineState) {
    throwIfFailed(device->CreateGraphicsPipelineState(&description, IID_PPV_ARGS(&pipelineState)));
}

D3D12_SHADER_BYTECODE PipelineState::loadAndCompileShader(const std::wstring &name, const std::string &target) {
    ID3DBlob *compiledShader = nullptr;
    ID3DBlob *errorBlob = nullptr;

    const auto path = std::wstring{SHADERS_PATH} + name;
    const auto result = D3DCompileFromFile(path.c_str(), nullptr, nullptr, "main", target.c_str(), 0, 0, &compiledShader, &errorBlob);
    throwIfFailed(result, errorBlob);

    this->shaderBlobs.emplace_back(compiledShader);
    return D3D12_SHADER_BYTECODE{compiledShader->GetBufferPointer(), compiledShader->GetBufferSize()};
}

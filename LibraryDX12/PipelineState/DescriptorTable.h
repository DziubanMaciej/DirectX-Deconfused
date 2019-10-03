#pragma once

#include "PipelineState/ShaderRegister.h"

#include <DXD/ExternalHeadersWrappers/d3d12.h>
#include <vector>

class DescriptorTable {
public:
    DescriptorTable(D3D12_SHADER_VISIBILITY shaderVisibility);
    DescriptorTable(DescriptorTable &&) = default;
    DescriptorTable &operator=(DescriptorTable &&) = default;
    DescriptorTable(const DescriptorTable &) = default;
    DescriptorTable &operator=(const DescriptorTable &) = default;

    DescriptorTable &appendCbvRange(CbvShaderRegister reg, UINT count);
    DescriptorTable &appendSrvRange(SrvShaderRegister reg, UINT count);
    DescriptorTable &appendUavRange(UavShaderRegister reg, UINT count);
    DescriptorTable &appendSamplerRange(SamplerShaderRegister reg, UINT count);

    D3D12_ROOT_PARAMETER1 getTable();

private:
    void compile();

    std::vector<D3D12_DESCRIPTOR_RANGE1> ranges = {};
    D3D12_ROOT_PARAMETER1 table = {};
    bool compiled = false;
};

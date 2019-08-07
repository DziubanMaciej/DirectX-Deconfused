#include "DescriptorTable.h"

DescriptorTable::DescriptorTable(D3D12_SHADER_VISIBILITY shaderVisibility) {
    table.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    table.ShaderVisibility = shaderVisibility;
}

DescriptorTable &DescriptorTable::appendCbvRange(CbvShaderRegister reg, UINT count) {
    D3D12_DESCRIPTOR_RANGE1 range{D3D12_DESCRIPTOR_RANGE_TYPE_CBV, count, reg, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND};
    this->ranges.push_back(std::move(range));
    return *this;
}

DescriptorTable &DescriptorTable::appendSrvRange(SrvShaderRegister reg, UINT count) {
    D3D12_DESCRIPTOR_RANGE1 range{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, count, reg, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND};
    this->ranges.push_back(std::move(range));
    return *this;
}

DescriptorTable &DescriptorTable::appendUavRange(UavShaderRegister reg, UINT count) {
    D3D12_DESCRIPTOR_RANGE1 range{D3D12_DESCRIPTOR_RANGE_TYPE_UAV, count, reg, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND};
    this->ranges.push_back(std::move(range));
    return *this;
}

DescriptorTable &DescriptorTable::appendSamplerRange(SamplerShaderRegister reg, UINT count) {
    D3D12_DESCRIPTOR_RANGE1 range{D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, count, reg, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND};
    this->ranges.push_back(std::move(range));
    return *this;
}

D3D12_ROOT_PARAMETER1 DescriptorTable::getTable() {
    compile();
    return table;
}

void DescriptorTable::compile() {
    if (!compiled) {
        table.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(ranges.size());
        table.DescriptorTable.pDescriptorRanges = ranges.data();
    }
}

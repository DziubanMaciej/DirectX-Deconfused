#pragma once

#include "DXD/ExternalHeadersWrappers/d3d12.h"
#include "DXD/ExternalHeadersWrappers/windows.h"

template <D3D12_DESCRIPTOR_RANGE_TYPE type>
struct ShaderRegister {
    explicit constexpr ShaderRegister(UINT value) : value(value) {}
    operator UINT() const { return value; }
    const UINT value;
};

using SamplerShaderRegister = ShaderRegister<D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER>;
using CbvShaderRegister = ShaderRegister<D3D12_DESCRIPTOR_RANGE_TYPE_CBV>;
using SrvShaderRegister = ShaderRegister<D3D12_DESCRIPTOR_RANGE_TYPE_SRV>;
using UavShaderRegister = ShaderRegister<D3D12_DESCRIPTOR_RANGE_TYPE_UAV>;

namespace ShaderRegisterHelpers {
inline SamplerShaderRegister s(UINT value) {
    return SamplerShaderRegister{value};
}

inline CbvShaderRegister b(UINT value) {
    return CbvShaderRegister{value};
}

inline SrvShaderRegister t(UINT value) {
    return SrvShaderRegister{value};
}

inline UavShaderRegister u(UINT value) {
    return UavShaderRegister{value};
}
} // namespace ShaderRegisterHelpers

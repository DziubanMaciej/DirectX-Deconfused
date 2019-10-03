#pragma once

#include <DXD/ExternalHeadersWrappers/d3d12.h>

class StaticSampler {
public:
    StaticSampler(D3D12_SHADER_VISIBILITY shaderVisibility) {
        filter(D3D12_FILTER_MIN_MAG_MIP_POINT);
        addressModeBorder(D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);
        lodRange(0, D3D12_FLOAT32_MAX);
        comparisonFunction(D3D12_COMPARISON_FUNC_NEVER);
        desc.ShaderVisibility = shaderVisibility;
    }

    StaticSampler &filter(D3D12_FILTER f) {
        desc.Filter = f;
        return *this;
    }

    StaticSampler &addressMode(D3D12_TEXTURE_ADDRESS_MODE mode) {
        desc.AddressU = mode;
        desc.AddressV = mode;
        desc.AddressW = mode;
        return *this;
    }

    StaticSampler &addressModeBorder(D3D12_STATIC_BORDER_COLOR borderColor) {
        addressMode(D3D12_TEXTURE_ADDRESS_MODE_BORDER);
        desc.BorderColor = borderColor;
        return *this;
    }

    StaticSampler &lodRange(float min, float max) {
        desc.MinLOD = min;
        desc.MaxLOD = max;
        return *this;
    }

    StaticSampler &comparisonFunction(D3D12_COMPARISON_FUNC func) {
        desc.ComparisonFunc = func;
        return *this;
    }

    const D3D12_STATIC_SAMPLER_DESC get() const {
        return desc;
    }

private:
    D3D12_STATIC_SAMPLER_DESC desc = {};
};

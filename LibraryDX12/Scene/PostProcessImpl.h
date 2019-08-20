#pragma once

#include "Resource/ConstantBuffer.h"
#include "Resource/ConstantBuffers.h"

#include "DXD/PostProcess.h"

// TODO std::variant to hold data as union?
class PostProcessImpl : public DXD::PostProcess {
protected:
    friend class DXD::PostProcess;
    PostProcessImpl() = default;

public:
    enum class Type {
        UNDEFINED,
        BLACK_BARS,
        CONVOLUTION
    };

    void setEnabled(bool enabled) override;

    void setBlackBars(float leftMarginPercent, float rightMarginPercent, float topMarginPercent, float bottomMarginPercent) override;

    void setConvolution(float divider, XMFLOAT3X3 matrix) override;
    void setConvolution(float divider,
                        float a00, float a01, float a02,
                        float a10, float a11, float a12,
                        float a20, float a21, float a22) override;

    auto isEnabled() const { return enabled; }
    auto getType() const { return type; }
    const auto &getDataBlackBars() { return *dataBlackBars; }
    auto &getDataConvolution() { return *dataConvolution; }

private:
    bool enabled = false;
    Type type = Type::UNDEFINED;

    struct DataBlackBars {
        float leftMarginPercent, rightMarginPercent, topMarginPercent, bottomMarginPercent;
    };
    std::unique_ptr<DataBlackBars> dataBlackBars = {};

    using DataConvolution = PostProcessConvolutionCB;
    std::unique_ptr<DataConvolution> dataConvolution = {};
};

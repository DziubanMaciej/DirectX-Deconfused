#pragma once

#include "DXD/Export.h"
#include "DXD/NonCopyableAndMovable.h"

#include "DXD/ExternalHeadersWrappers/DirectXMath.h"
#include <memory>

namespace DXD {

class Light;
class Object;
class Camera;

class EXPORT PostProcess : NonCopyableAndMovable {
public:
    virtual ~PostProcess() = default;
    static std::unique_ptr<PostProcess> create();

    virtual void setEnabled(bool enabled) = 0;
    virtual bool isEnabled() const = 0;

    virtual void setBlackBars(float leftMarginPercent, float rightMarginPercent, float topMarginPercent, float bottomMarginPercent) = 0;

    virtual void setConvolution(float divider, XMFLOAT3X3 matrix) = 0;
    virtual void setConvolution(float divider,
                                float a00, float a01, float a02,
                                float a10, float a11, float a12,
                                float a20, float a21, float a22) = 0;
    virtual void setConvolutionSharpen() = 0;
    virtual void setConvolutionGaussianBlur() = 0;
    virtual void setConvolutionEdgeDetection() = 0;

    virtual void setLinearColorCorrection(XMFLOAT3X3 matrix) = 0;
    virtual void setLinearColorCorrection(float a00, float a01, float a02,
                                          float a10, float a11, float a12,
                                          float a20, float a21, float a22) = 0;
    virtual void setLinearColorCorrectionSepia() = 0;

    virtual void setGaussianBlur(UINT passCount, UINT samplingRange) = 0;

protected:
    PostProcess() = default;
};

} // namespace DXD

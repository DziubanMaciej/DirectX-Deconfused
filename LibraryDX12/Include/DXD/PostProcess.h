#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <DXD/ExternalHeadersWrappers/DirectXMath.h>
#include <memory>

namespace DXD {

class Light;
class Object;
class Camera;

/// \brief 2D effect applied on rendered scene
///
/// Object representing post processing effect added to the raster image. PostProcess objects
/// form an effect chain inside the Scene object and are applied in order specified by the
/// user. After creation Application should call one of functions setting the effect, e.g.
/// setConvolution, setConvolutionSharpen. These functions automatically enable the effect.
class EXPORT PostProcess : NonCopyableAndMovable {
public:
    /// @{

    /// Enables or disables the effect. Enabling a PostProcess object which has never been set
    /// to any effect is undefined and may fail.
    /// \param enabled desired state of the effect object to set
    virtual void setEnabled(bool enabled) = 0;
    virtual bool isEnabled() const = 0;
    /// @}

    /// @{
    virtual void setBlackBars(float leftMarginPercent, float rightMarginPercent, float topMarginPercent, float bottomMarginPercent) = 0;
    /// @}

    /// \name Convolution filter
    /// \brief Apply convolution filter multiplying each pixel and its neighbouring pixels by the
    /// specified matrix. Helper functions for common filters are provided.
    /// @{
    virtual void setConvolution(float divider, XMFLOAT3X3 matrix) = 0;
    virtual void setConvolution(float divider,
                                float a00, float a01, float a02,
                                float a10, float a11, float a12,
                                float a20, float a21, float a22) = 0;
    virtual void setConvolutionSharpen() = 0;
    virtual void setConvolutionGaussianBlur() = 0;
    virtual void setConvolutionEdgeDetection() = 0;
    /// @}

    /// \name Linear color correction
    /// \brief Multiple given matrix and each pixel of the raster image interpreted as a column vector.
    /// Helper function for common matrices are provided.
    /// @{
    virtual void setLinearColorCorrection(XMFLOAT3X3 matrix) = 0;
    virtual void setLinearColorCorrection(float a00, float a01, float a02,
                                          float a10, float a11, float a12,
                                          float a20, float a21, float a22) = 0;
    virtual void setLinearColorCorrectionSepia() = 0;
    /// @}

    /// \name Scene blurring
    /// @{
    virtual void setGaussianBlur(UINT passCount, UINT samplingRange) = 0;
    /// @}

    /// Factory method used to create PostProcess instances. Note that inactive instances are created.
    /// Application has to use one of "set" functions to set the effect and enable it. Calling setEnable
    /// before setting an effect is undefined behaviour.
    /// \return PostProcess instance
    static std::unique_ptr<PostProcess> create();
    virtual ~PostProcess() = default;

protected:
    PostProcess() = default;
};

} // namespace DXD

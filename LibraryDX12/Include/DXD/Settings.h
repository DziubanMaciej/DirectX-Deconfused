#pragma once
#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

namespace DXD {

/// \brief Global effects configuration
///
/// Provides easy to use set/get interface for altering global settings used by
/// the engine.
class EXPORT Settings : NonCopyableAndMovable {
public:
    virtual ~Settings() = default;

    /// \name Vertical synchronization
    /// \brief Synchronize rendering with monitor's refresh rate.
    /// @{
    virtual void setVerticalSyncEnabled(bool value) = 0;
    virtual bool getVerticalSyncEnabled() const = 0;
    /// @}

    /// \name Screen space ambient occlusion switch
    /// \brief This effect provides subtle shadows render in some cavities found
    /// internally by the engine based on depth buffer.
    /// @{
    virtual void setSsaoEnabled(bool value) = 0;
    virtual bool getSsaoEnabled() const = 0;
    /// @}

    /// \name Bloom effect for bright lights
    /// @{
    virtual void setBloomEnabled(bool value) = 0;
    virtual bool getBloomEnabled() const = 0;
    /// @}

    /// \name Screen space reflections
    /// \brief This effect provides reflections, intensity of which scales with
    /// specularity of Object instances being rendered.
    /// @{
    virtual void setSsrEnabled(bool value) = 0;
    virtual bool getSsrEnabled() const = 0;
    /// @}

    /// \name Fog
    /// \brief This effect provides fog.
    /// @{
    virtual void setFogEnabled(bool value) = 0;
    virtual bool getFogEnabled() const = 0;
    /// @}

    /// \name Dof
    /// \brief This effect provides depth of field.
    /// @{
    virtual void setDofEnabled(bool value) = 0;
    virtual bool getDofEnabled() const = 0;
    /// @}

    /// \name Shadows quality
    /// \brief This effect casts shadows of Objects based on Light sources. Quality
    /// of the effect means resolution of shadow maps used internally and number of
    /// samples taken in shaders.
    /// @{

    /// \param value shadows quality between 0 to 10 (inclusive). Values exceeding this range are clamped.
    virtual void setShadowsQuality(unsigned int value) = 0;
    virtual unsigned int getShadowsQuality() const = 0;
    /// @}
};
} // namespace DXD

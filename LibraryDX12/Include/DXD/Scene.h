#pragma once

#include "DXD/Utility/Export.h"
#include "DXD/Utility/NonCopyableAndMovable.h"

#include <memory>

namespace DXD {

class Text;
class Light;
class Object;
class Camera;
class PostProcess;
class Sprite;

/// \brief Container for all the elements drawn
///
/// Provides interface for adding and removing all entities drawn or tied to drawing
/// such as objects, post processes or sprites. Scene can be bound to Window instance
/// so that it is rendered every frame.
class EXPORT Scene : NonCopyableAndMovable {
public:
    /// @{
    virtual void setBackgroundColor(float r, float g, float b) = 0;
    virtual void setAmbientLight(float r, float g, float b) = 0;
    /// @}

    /// @{
    virtual void addLight(DXD::Light &light) = 0;
    virtual bool removeLight(DXD::Light &light) = 0;
    /// @}

    /// @{
    virtual void addPostProcess(DXD::PostProcess &postProcess) = 0;
    virtual bool removePostProcess(DXD::PostProcess &postProcess) = 0;
    /// @}

    /// @{
    virtual void addObject(DXD::Object &object) = 0;
    virtual bool removeObject(DXD::Object &object) = 0;
    /// @}

    /// @{
    virtual void addText(DXD::Text &text) = 0;
    virtual bool removeText(DXD::Text &text) = 0;
    /// @}

    /// @{
    virtual void addSprite(DXD::Sprite &sprite) = 0;
    virtual bool removeSprite(DXD::Sprite &sprite) = 0;
    /// @}

    /// @{
    virtual void setCamera(DXD::Camera &camera) = 0;
    virtual DXD::Camera *getCamera() = 0;
    /// @}

    /// Factory function for creating the Scene instance
    /// \param application execution for the object
    static std::unique_ptr<Scene> create(DXD::Application &application);
    virtual ~Scene() = default;

protected:
    Scene() = default;
};

} // namespace DXD

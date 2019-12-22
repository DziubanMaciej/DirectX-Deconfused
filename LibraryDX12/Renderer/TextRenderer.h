#pragma once

#include "DXD/Utility/NonCopyableAndMovable.h"

class D2DWrappedResource;
class SceneImpl;

class TextRenderer : DXD::NonCopyableAndMovable {
public:
    TextRenderer(SceneImpl &scene);

    void renderTexts(D2DWrappedResource &output);
    bool isEnabled() const;

private:
    SceneImpl &scene;
};

#pragma once
#include "raylib.h"

namespace res
{
    struct RenderableComponent
    {
    };

    struct ModelComponent
    {
        Model model;
    };

    struct CameraComponent
    {
        Camera3D raylibCamera;
    };

    struct SpherePrimitiveComponent
    {

    };
}

#pragma once
#include "raylib.h"

namespace res
{
    struct cRenderable
    {
    };

    struct cModel
    {
        Model model;
    };

    struct cCamera
    {
        Camera3D raylibCamera;
    };
}

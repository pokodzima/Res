#pragma once
#include "raylib.h"

namespace res
{
    struct cPosition
    {
        float x, y, z;
    };

    struct cRotation
    {
        float x, y, z, w;
    };

    struct cMatrix
    {
        Matrix matrix;
    };
}

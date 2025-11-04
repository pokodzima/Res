#pragma once

#include <raylib.h>
#include <raymath.h>

namespace res
{
    struct MatrixComponent
    {
        Matrix matrix{MatrixIdentity()};
    };

    struct DebugCameraMovementComponent {
        int movement_type;
    };
}

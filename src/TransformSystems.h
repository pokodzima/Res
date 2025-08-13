#pragma once

#include <iostream>

#include "Phases.h"
#include "TransformComponents.h"
#include "MathUtils.h"

#include "flecs.h"


namespace res
{
    struct TransformSystems
    {
        explicit TransformSystems(flecs::world& world);
    };
}

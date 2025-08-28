#include "TransformSystems.h"

#include <flecs.h>

res::TransformSystems::TransformSystems(flecs::world& world)
{
    world.module<TransformSystems>();
}

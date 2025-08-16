#pragma once

namespace flecs
{
    struct world;
}

namespace res
{
    struct PhysicsSystems
    {
        explicit PhysicsSystems(flecs::world& world);
    };
}

#pragma once

namespace flecs
{
    struct world;
}

namespace res
{
    struct InputSystems
    {
        explicit InputSystems(flecs::world& world);
    };
}

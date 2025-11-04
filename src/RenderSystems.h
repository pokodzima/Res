#pragma once

namespace flecs
{
    struct world;
}

namespace res
{
    struct RenderSystems
    {
        explicit RenderSystems(flecs::world& world);
    };
}

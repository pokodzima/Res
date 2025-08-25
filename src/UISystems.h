#pragma once

namespace flecs
{
    struct world;
}

namespace res
{
    struct UISystems
    {
        explicit UISystems(flecs::world& world);
    };
}

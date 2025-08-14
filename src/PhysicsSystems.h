#pragma once

namespace res
{
    struct cModel;
}

namespace JPH
{
    class StaticCompoundShapeSettings;
}

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

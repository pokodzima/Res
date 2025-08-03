#pragma once
#include <flecs.h>

namespace res
{
    static flecs::entity OnBegin;
    static flecs::entity OnTick;
    static flecs::entity OnPostTick;
    static flecs::entity OnPreRender;
    static flecs::entity OnRender;
    static flecs::entity OnPreRender3D;
    static flecs::entity OnRender3D;
    static flecs::entity OnPostRender3D;
    static flecs::entity OnRender2D;
    static flecs::entity OnPostRender;

    struct Phases
    {
        explicit Phases(flecs::world& world)
        {
            // ReSharper disable once CppExpressionWithoutSideEffects
            world.module<Phases>();\

            OnBegin = world.entity()
                           .add(flecs::Phase)
                           .depends_on(flecs::OnStart);

            OnTick = world.entity()
                          .add(flecs::Phase)
                          .depends_on(flecs::OnUpdate);

            OnPostTick = world.entity()
                              .add(flecs::Phase)
                              .depends_on(OnTick);

            OnPreRender = world.entity()
                               .add(flecs::Phase)
                               .depends_on(OnPostTick);

            OnRender = world.entity()
                            .add(flecs::Phase)
                            .depends_on(OnPreRender);

            OnPreRender3D = world.entity()
                                 .add(flecs::Phase)
                                 .depends_on(OnRender);

            OnRender3D = world.entity()
                              .add(flecs::Phase)
                              .depends_on(OnPreRender3D);

            OnPostRender3D = world.entity()
                                  .add(flecs::Phase)
                                  .depends_on(OnRender3D);

            OnRender2D = world.entity()
                              .add(flecs::Phase)
                              .depends_on(OnRender3D);

            OnPostRender = world.entity()
                                .add(flecs::Phase)
                                .depends_on(OnRender2D);
        }
    };
}

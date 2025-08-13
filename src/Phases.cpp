#include "Phases.h"
#include <flecs.h>

void res::CreatePhases(flecs::world& world)
{
    auto onBegin = world.entity(OnBeginPhaseName.data())
                        .add(flecs::Phase)
                        .depends_on(flecs::OnStart);

    auto onTick = world.entity(OnTickPhaseName.data())
                       .add(flecs::Phase)
                       .depends_on(flecs::OnUpdate);

    auto onPostTick = world.entity(OnPostTickPhaseName.data())
                           .add(flecs::Phase)
                           .depends_on(onTick);

    auto onPreRender = world.entity(OnPreRenderPhaseName.data())
                            .add(flecs::Phase)
                            .depends_on(onPostTick);

    auto onRender = world.entity(OnRenderPhaseName.data())
                         .add(flecs::Phase)
                         .depends_on(onPreRender);

    auto onPreRender3D = world.entity(OnPreRender3DPhaseName.data())
                              .add(flecs::Phase)
                              .depends_on(onRender);

    auto onRender3D = world.entity(OnRender3DPhaseName.data())
                           .add(flecs::Phase)
                           .depends_on(onPreRender3D);

    auto onPostRender3D = world.entity(OnPostRender3DPhaseName.data())
                               .add(flecs::Phase)
                               .depends_on(onRender3D);

    auto onRender2D = world.entity(OnRender2DPhaseName.data())
                           .add(flecs::Phase)
                           .depends_on(onRender3D);

    auto onPostRender = world.entity(OnPostRenderPhaseName.data())
                             .add(flecs::Phase)
                             .depends_on(onRender2D);
}

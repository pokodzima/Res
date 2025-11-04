#include "Phases.h"
#include <flecs.h>

void res::CreatePhases(flecs::world& world)
{
    auto on_begin = world.entity(OnBeginPhaseName.data())
                        .add(flecs::Phase)
                        .depends_on(flecs::OnStart);

    auto on_tick = world.entity(OnTickPhaseName.data())
                       .add(flecs::Phase)
                       .depends_on(flecs::OnUpdate);

    auto on_post_tick = world.entity(OnPostTickPhaseName.data())
                           .add(flecs::Phase)
                           .depends_on(on_tick);

    auto on_pre_render = world.entity(OnPreRenderPhaseName.data())
                            .add(flecs::Phase)
                            .depends_on(on_post_tick);

    auto on_render = world.entity(OnRenderPhaseName.data())
                         .add(flecs::Phase)
                         .depends_on(on_pre_render);

    auto on_pre_render_3d = world.entity(OnPreRender3DPhaseName.data())
                              .add(flecs::Phase)
                              .depends_on(on_render);

    auto on_render_3d = world.entity(OnRender3DPhaseName.data())
                           .add(flecs::Phase)
                           .depends_on(on_pre_render_3d);

    auto on_post_render_3d = world.entity(OnPostRender3DPhaseName.data())
                               .add(flecs::Phase)
                               .depends_on(on_render_3d);

    auto on_render_2d = world.entity(OnRender2DPhaseName.data())
                           .add(flecs::Phase)
                           .depends_on(on_render_3d);

    auto on_post_render = world.entity(OnPostRenderPhaseName.data())
                             .add(flecs::Phase)
                             .depends_on(on_render_2d);
}

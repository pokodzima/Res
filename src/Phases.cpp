#include "Phases.h"

#include <flecs.h>

void res::CreatePhases(flecs::world& world)
{
    auto on_begin = world.entity(kBeginPhaseName.data())
                        .add(flecs::Phase)
                        .depends_on(flecs::OnStart);

    auto on_tick = world.entity(kTickPhaseName.data())
                       .add(flecs::Phase)
                       .depends_on(flecs::OnUpdate);

    auto on_post_tick = world.entity(kPostTickPhaseName.data())
                           .add(flecs::Phase)
                           .depends_on(on_tick);

    auto on_pre_render = world.entity(kPreRenderPhaseName.data())
                            .add(flecs::Phase)
                            .depends_on(on_post_tick);

    auto on_render = world.entity(kRenderPhaseName.data())
                         .add(flecs::Phase)
                         .depends_on(on_pre_render);

    auto on_pre_render_3d = world.entity(kPreRender3DPhaseName.data())
                              .add(flecs::Phase)
                              .depends_on(on_render);

    auto on_render_3d = world.entity(kRender3DPhaseName.data())
                           .add(flecs::Phase)
                           .depends_on(on_pre_render_3d);

    auto on_post_render_3d = world.entity(kPostRender3DPhaseName.data())
                               .add(flecs::Phase)
                               .depends_on(on_render_3d);

    auto on_render_2d = world.entity(kRender2DPhaseName.data())
                           .add(flecs::Phase)
                           .depends_on(on_render_3d);

    auto on_post_render = world.entity(kPostRenderPhaseName.data())
                             .add(flecs::Phase)
                             .depends_on(on_render_2d);
}

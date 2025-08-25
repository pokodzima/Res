#include "DebugSystems.h"

#include "RenderComponents.h"
#include "TransformComponents.h"
#include "MathUtils.h"
#include "PhysicsComponents.h"

#include <rlImGui.h>
#include <flecs.h>
#include <imgui.h>

#include <cassert>


res::DebugSystems::DebugSystems(flecs::world& world)
{
    world.module<DebugSystems>();

    auto onRender2DPhase = world.lookup(OnRender2DPhaseName.data());
    auto onPreRenderPhase = world.lookup(OnPreRenderPhaseName.data());

    assert(onRender2DPhase != 0 && "OnRender2DPhase not found");
    assert(onPreRenderPhase != 0 && "OnPreRenderPhase not found");

    world.system("Render ImGui")
         .kind(onRender2DPhase)
         .run([&world](flecs::iter it)
         {
             rlImGuiBegin();

             const auto character = world.lookup("Character");
             if (character.is_alive())
             {
                 if (const auto matrixComponent = character.try_get<cMatrix>())
                 {
                     const auto [x, y, z] = GetPositionFromMatrix(matrixComponent->matrix);
                     ImGui::Text("%f,%f,%f", x, y, z);
                 }
                 else
                 {
                     ImGui::Text("Character has no Matrix Component");
                 }
             }
             else
             {
                 ImGui::Text("Character not found");
             }

             rlImGuiEnd();
         });

    world.system<cCamera>("Debug Camera Movement")
         .kind(onPreRenderPhase)
         .each([](cCamera& c)
         {
             //     UpdateCamera(&c.raylibCamera, CAMERA_FREE);
         });
}

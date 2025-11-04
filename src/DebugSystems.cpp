#include "DebugSystems.h"

#include "RenderComponents.h"
#include "TransformComponents.h"
#include "MathUtils.h"
#include "PhysicsComponents.h"

#include <rlImGui.h>
#include <flecs.h>
#include <imgui.h>

#include <cassert>

#include "InputComponents.h"


res::DebugSystems::DebugSystems(flecs::world& world)
{
    world.module<DebugSystems>();

    auto on_render_2d_phase = world.lookup(kRender2DPhaseName.data());
    auto on_pre_render_phase = world.lookup(kPreRenderPhaseName.data());

    assert(on_render_2d_phase != 0 && "OnRender2DPhase not found");
    assert(on_pre_render_phase != 0 && "OnPreRenderPhase not found");

    world.system("Render ImGui")
         .kind(on_render_2d_phase)
         .run([&world](flecs::iter it)
         {
             rlImGuiBegin();

             const auto character = world.lookup("Character");
             if (character != 0 && character.is_alive())
             {
                 if (const auto matrix_component = character.try_get<MatrixComponent>())
                 {
                     const auto [x, y, z] = GetPositionFromMatrix(matrix_component->matrix);
                     const auto input = character.get<MovementInputComponent>().input;
                     ImGui::Text("Player Position: %f,%f,%f", x, y, z);
                     ImGui::Text("Movement Input: %f,%f", input.x, input.y);
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
}

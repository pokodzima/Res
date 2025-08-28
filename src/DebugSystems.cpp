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
                 if (const auto matrixComponent = character.try_get<MatrixComponent>())
                 {
                     const auto [x, y, z] = GetPositionFromMatrix(matrixComponent->matrix);
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

    world.system<CameraComponent>("Debug Camera Movement")
         .kind(onPreRenderPhase)
         .each([](CameraComponent& c)
         {
             //     UpdateCamera(&c.raylibCamera, CAMERA_FREE);
         });
}

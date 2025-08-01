#pragma once
#include "Phases.h"
#include "RenderComponents.h"

#include "rlImGui.h"
#include "flecs.h"
#include "imgui.h"

namespace res
{
    struct DebugSystems
    {
        explicit DebugSystems(flecs::world& world)
        {
            // ReSharper disable once CppExpressionWithoutSideEffects
            world.module<DebugSystems>();

            world.system<const cCamera>("OnRenderImGUI")
                 .kind(OnRender2D)
                 .each([](const cCamera& c)
                 {
                     rlImGuiBegin();
                     const auto camera = c.raylibCamera;
                     ImGui::Text("%f,%f,%f", camera.position.x, camera.position.y, camera.position.z);
                     rlImGuiEnd();
                 });
        }
    };
}

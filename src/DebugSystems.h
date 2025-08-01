#pragma once
#include "Phases.h"
#include "RenderComponents.h"
#include "TransformComponents.h"

#include "rlImGui.h"
#include "flecs.h"
#include "imgui.h"
#include "MathUtils.h"


namespace res
{
    struct DebugSystems
    {
        explicit DebugSystems(flecs::world& world)
        {
            // ReSharper disable once CppExpressionWithoutSideEffects
            world.module<DebugSystems>();

            world.system<const cCamera, const cMatrix>("OnRenderImGUI")
                 .kind(OnRender2D)
                 .each([](const cCamera& c, const cMatrix& m)
                 {
                     rlImGuiBegin();
                     const auto camera = c.raylibCamera;
                     ImGui::Text("%f,%f,%f", camera.position.x, camera.position.y, camera.position.z);
                     ImGui::Text("%f,%f,%f", camera.up.x, camera.up.y, camera.up.z);
                     ImGui::Text("%f,%f,%f", camera.target.x, camera.target.y, camera.target.z);
                     rlImGuiEnd();
                 });

            world.system<cCamera>("DebugCameraMovement")
                 .kind(OnPreRender)
                 .each([](cCamera& c)
                 {
                     return;
                     UpdateCamera(&c.raylibCamera, CAMERA_FREE);
                 });
        }
    };
}

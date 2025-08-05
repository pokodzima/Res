#pragma once
#include "Phases.h"
#include "RenderComponents.h"
#include "TransformComponents.h"

#include "rlImGui.h"
#include "flecs.h"
#include "imgui.h"
#include "MathUtils.h"
#include "PhysicsComponents.h"


namespace res
{
    struct DebugSystems
    {
        explicit DebugSystems(flecs::world& world)
        {
            // ReSharper disable once CppExpressionWithoutSideEffects
            world.module<DebugSystems>();

            world.system<const cMeshCollider, const cMatrix>("OnRenderImGUI")
                 .kind(OnRender2D)
                 .each([](const cMeshCollider& mc, const cMatrix& m)
                 {
                     rlImGuiBegin();
                     auto pos = GetPositionFromMatrix(m.matrix);
                     ImGui::Text("%f,%f,%f", pos.x, pos.y, pos.z);
                     rlImGuiEnd();
                 });

            world.system<cCamera>("DebugCameraMovement")
                 .kind(OnPreRender)
                 .each([](cCamera& c)
                 {
                     UpdateCamera(&c.raylibCamera, CAMERA_FREE);
                 });
        }
    };
}

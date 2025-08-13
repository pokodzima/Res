#include "DebugSystems.h"

#include "RenderComponents.h"
#include "TransformComponents.h"
#include "MathUtils.h"
#include "PhysicsComponents.h"

#include <rlImGui.h>
#include <flecs.h>
#include <imgui.h>


res::DebugSystems::DebugSystems(flecs::world& world)
{
    world.module<DebugSystems>();
    auto onRender2DPhase = world.lookup(OnRender2DPhaseName.data());
    auto OnPreRenderPhase = world.lookup(OnPreRenderPhaseName.data());

    world.system<const cMeshCollider, const cMatrix>("Render ImGui")
         .kind(onRender2DPhase)
         .each([](const cMeshCollider& mc, const cMatrix& m)
         {
             rlImGuiBegin();
             auto pos = GetPositionFromMatrix(m.matrix);
             ImGui::Text("%f,%f,%f", pos.x, pos.y, pos.z);
             rlImGuiEnd();
         });

    world.system<cCamera>("Debug Camera Movement")
         .kind(OnPreRenderPhase)
         .each([](cCamera& c)
         {
             UpdateCamera(&c.raylibCamera, CAMERA_FREE);
         });
}

#pragma once
#include "Phases.h"
#include "RenderComponents.h"

#include "raylib.h"
#include "flecs.h"
#include "raymath.h"


namespace res
{
    struct RenderSystems
    {
        explicit RenderSystems(flecs::world& world)
        {
            // ReSharper disable once CppExpressionWithoutSideEffects
            world.module<RenderSystems>();

            world.system("BeginRender")
                 .kind(OnPreRender)
                 .run([](flecs::iter& it)
                 {
                     BeginDrawing();
                     ClearBackground(WHITE);
                 });

            world.system<const cCamera>("BeginRender3D")
                 .kind(OnPreRender3D)
                 .each([](const cCamera& c)
                 {
                     const auto camera = c.raylibCamera;
                     BeginMode3D(camera);
                 });

            world.system("EndRender3D")
                 .kind(OnPostRender3D)
                 .run([](flecs::iter& it)
                 {
                     EndMode3D();
                 });

            world.system("DrawFPS")
                 .kind(OnRender2D)
                 .run([](flecs::iter& it)
                 {
                     DrawFPS(20, 20);
                 });

            world.system("EndRender")
                 .kind(OnPostRender)
                 .run([](flecs::iter& it)
                 {
                     EndDrawing();
                 });

            world.system<const cRenderable, const cModel>()
                 .kind(OnRender3D)
                 .each([](const cRenderable& r, const cModel& mh)
                 {
                     DrawModel(mh.model, Vector3Zero(), 0.01f, WHITE);
                 });
        }
    };
}

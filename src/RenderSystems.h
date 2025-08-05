#pragma once
#include "Phases.h"
#include "RenderComponents.h"
#include "TransformComponents.h"
#include "MathUtils.h"

#include <raylib.h>
#include <flecs.h>
#include <raymath.h>


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

            world.system<const cRenderable, const cModel, const cMatrix>()
                 .kind(OnRender3D)
                 .each([](const cRenderable& r, const cModel& mh, const cMatrix matrix)
                 {
                     DrawModel(mh.model, GetPositionFromMatrix(matrix.matrix), 1.0f, WHITE);
                 });

            world.system<const cRenderable, const cRlSphere, const cMatrix>()
                 .kind(OnRender3D)
                 .each([](const cRenderable& r, const cRlSphere& rlSphere, const cMatrix matrix)
                 {
                     DrawSphere(GetPositionFromMatrix(matrix.matrix), 0.5f,RED);
                     DrawGrid(20, 0.5);
                 });

            world.system<cCamera, const cMatrix>()
                 .kind(OnBegin)
                 .each([](cCamera& cameraComponent, const cMatrix& matrixComponent)
                 {
                     return;
                     cameraComponent.raylibCamera.position = GetPositionFromMatrix(matrixComponent.matrix);
                     cameraComponent.raylibCamera.up = GetUpVector(matrixComponent.matrix);
                     cameraComponent.raylibCamera.target = GetTargetForCamera(matrixComponent.matrix);
                 });
        }
    };
}

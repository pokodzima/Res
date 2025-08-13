#include "RenderSystems.h"

#include "MathUtils.h"
#include "Phases.h"
#include "RenderComponents.h"
#include "TransformComponents.h"

#include <flecs.h>
#include <raylib.h>
#include <raymath.h>

res::RenderSystems::RenderSystems(flecs::world& world)
{
    world.module<RenderSystems>();

    auto OnPreRenderPhase = world.lookup(OnPreRenderPhaseName.data());
    auto OnPreRender3DPhase = world.lookup(OnPreRender3DPhaseName.data());
    auto OnPostRender3DPhase = world.lookup(OnPostRender3DPhaseName.data());
    auto OnRender2DPhase = world.lookup(OnRender2DPhaseName.data());
    auto OnPostRenderPhase = world.lookup(OnPostRenderPhaseName.data());
    auto OnRender3DPhase = world.lookup(OnRender3DPhaseName.data());
    auto OnBeginPhase = world.lookup(OnBeginPhaseName.data());


    world.system("Begin Render")
         .kind(OnPreRenderPhase)
         .run([](flecs::iter& it)
         {
             BeginDrawing();
             ClearBackground(WHITE);
         });

    world.system<const cCamera>("Begin Render3D")
         .kind(OnPreRender3DPhase)
         .each([](const cCamera& c)
         {
             const auto camera = c.raylibCamera;
             BeginMode3D(camera);
         });

    world.system("End Render3D")
         .kind(OnPostRender3DPhase)
         .run([](flecs::iter& it)
         {
             EndMode3D();
         });

    world.system("Draw FPS")
         .kind(OnRender2DPhase)
         .run([](flecs::iter& it)
         {
             DrawFPS(20, 20);
         });

    world.system("End Render")
         .kind(OnPostRenderPhase)
         .run([](flecs::iter& it)
         {
             EndDrawing();
         });

    world.system<const cRenderable, const cModel, const cMatrix>("Render Models")
         .kind(OnRender3DPhase)
         .each([](const cRenderable& r, const cModel& mh, const cMatrix matrix)
         {
             DrawModel(mh.model, GetPositionFromMatrix(matrix.matrix), 1.0f, WHITE);
         });

    world.system<const cRenderable, const cRlSphere, const cMatrix>("Draw Spheres")
         .kind(OnRender3DPhase)
         .each([](const cRenderable& r, const cRlSphere& rlSphere, const cMatrix matrix)
         {
             DrawSphere(GetPositionFromMatrix(matrix.matrix), 0.5f,RED);
             DrawGrid(20, 0.5);
         });

    world.system<cCamera, const cMatrix>()
         .kind(OnBeginPhase)
         .each([](cCamera& cameraComponent, const cMatrix& matrixComponent)
         {
             return;
             cameraComponent.raylibCamera.position = GetPositionFromMatrix(matrixComponent.matrix);
             cameraComponent.raylibCamera.up = GetUpVector(matrixComponent.matrix);
             cameraComponent.raylibCamera.target = GetTargetForCamera(matrixComponent.matrix);
         });
}

#include "RenderSystems.h"

#include "MathUtils.h"
#include "Phases.h"
#include "RenderComponents.h"
#include "TransformComponents.h"

#include <flecs.h>
#include <raylib.h>
#include <raymath.h>

res::RenderSystems::RenderSystems(flecs::world &world) {
  world.module<RenderSystems>();

  auto OnPreRenderPhase = world.lookup(OnPreRenderPhaseName.data());
  auto OnPreRender3DPhase = world.lookup(OnPreRender3DPhaseName.data());
  auto OnPostRender3DPhase = world.lookup(OnPostRender3DPhaseName.data());
  auto OnRender2DPhase = world.lookup(OnRender2DPhaseName.data());
  auto OnPostRenderPhase = world.lookup(OnPostRenderPhaseName.data());
  auto OnRender3DPhase = world.lookup(OnRender3DPhaseName.data());
  auto OnBeginPhase = world.lookup(OnBeginPhaseName.data());

  assert(OnPreRenderPhase != 0 && "OnPreRenderPhase not found!");
  assert(OnPreRender3DPhase != 0 && "OnPreRender3DPhase not found!");
  assert(OnPostRender3DPhase != 0 && "OnPostRender3DPhase not found!");
  assert(OnRender2DPhase != 0 && "OnRender2DPhase not found!");
  assert(OnPostRenderPhase != 0 && "OnPostRenderPhase not found!");
  assert(OnRender3DPhase != 0 && "OnRender3DPhase not found!");
  assert(OnBeginPhase != 0 && "OnBeginPhase not found!");

  world.system("Begin Render").kind(OnPreRenderPhase).run([](flecs::iter &it) {
    BeginDrawing();
    ClearBackground(WHITE);
  });

  world.system<const CameraComponent>("Begin Render3D")
      .kind(OnPreRender3DPhase)
      .each([](const CameraComponent &c) {
        const auto camera = c.raylibCamera;
        BeginMode3D(camera);
      });

  world.system("End Render3D")
      .kind(OnPostRender3DPhase)
      .run([](flecs::iter &it) { EndMode3D(); });

  world.system("Draw FPS").kind(OnRender2DPhase).run([](flecs::iter &it) {
    DrawFPS(20, 20);
  });

  world.system("End Render").kind(OnPostRenderPhase).run([](flecs::iter &it) {
    EndDrawing();
  });

  world
      .system<const RenderableComponent, const ModelComponent,
              const MatrixComponent>("Render Models")
      .kind(OnRender3DPhase)
      .each([](const RenderableComponent &r, const ModelComponent &mh,
               const MatrixComponent matrix) {
        DrawModel(mh.model, GetPositionFromMatrix(matrix.matrix), 1.0f, WHITE);
      });

  world
      .system<const RenderableComponent, const SpherePrimitiveComponent,
              const MatrixComponent>("Draw Spheres")
      .kind(OnRender3DPhase)
      .each([](const RenderableComponent &r,
               const SpherePrimitiveComponent &spherePrimitiveComponent,
               const MatrixComponent matrix) {
        DrawSphere(GetPositionFromMatrix(matrix.matrix), 0.5f, RED);
      });

  world
      .system<const RenderableComponent, const CapsulePrimitiveComponent,
              const MatrixComponent>("Draw Capsules")
      .kind(OnRender3DPhase)
      .each([](const RenderableComponent &r,
               const CapsulePrimitiveComponent &capsulePrimitiveComponent,
               const MatrixComponent matrix) {
        auto startPosition = GetPositionFromMatrix(matrix.matrix);
        startPosition.y -= 1.0f;
        auto endPosition = startPosition;
        endPosition.y += 2.0f;
        DrawCapsule(startPosition, endPosition, 0.5f, 8, 8, RED);
      });

  world
      .system<const RenderableComponent, const GridPrimitiveComponent>(
          "Draw Grid")
      .kind(OnRender3DPhase)
      .each([](const RenderableComponent &r, const GridPrimitiveComponent &g) {
        DrawGrid(g.slices, g.spacing);
      });

  world.system<CameraComponent, const MatrixComponent>()
      .kind(OnBeginPhase)
      .each([](CameraComponent &cameraComponent,
               const MatrixComponent &matrixComponent) {
        return;
        cameraComponent.raylibCamera.position =
            GetPositionFromMatrix(matrixComponent.matrix);
        cameraComponent.raylibCamera.up = GetUpVector(matrixComponent.matrix);
        cameraComponent.raylibCamera.target =
            GetTargetForCamera(matrixComponent.matrix);
      });
}

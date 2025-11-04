#include "RenderSystems.h"

#include <flecs.h>
#include <raylib.h>
#include <raymath.h>

#include "MathUtils.h"
#include "Phases.h"
#include "RenderComponents.h"
#include "TransformComponents.h"

res::RenderSystems::RenderSystems(flecs::world &world) {
  world.module<RenderSystems>();

  auto on_pre_render_phase = world.lookup(kPreRenderPhaseName.data());
  auto on_pre_render_3d_phase = world.lookup(kPreRender3DPhaseName.data());
  auto on_post_render_3d_phase = world.lookup(kPostRender3DPhaseName.data());
  auto on_render_2d_phase = world.lookup(kRender2DPhaseName.data());
  auto on_post_render_phase = world.lookup(kPostRenderPhaseName.data());
  auto on_render_3d_phase = world.lookup(kRender3DPhaseName.data());
  auto on_begin_phase = world.lookup(kBeginPhaseName.data());

  assert(on_pre_render_phase != 0 && "OnPreRenderPhase not found!");
  assert(on_pre_render_3d_phase != 0 && "OnPreRender3DPhase not found!");
  assert(on_post_render_3d_phase != 0 && "OnPostRender3DPhase not found!");
  assert(on_render_2d_phase != 0 && "OnRender2DPhase not found!");
  assert(on_post_render_phase != 0 && "OnPostRenderPhase not found!");
  assert(on_render_3d_phase != 0 && "OnRender3DPhase not found!");
  assert(on_begin_phase != 0 && "OnBeginPhase not found!");

  world.system("Begin Render").kind(on_pre_render_phase).run([](flecs::iter &it) {
    BeginDrawing();
    ClearBackground(WHITE);
  });

  world.system<const CameraComponent>("Begin Render3D")
      .kind(on_pre_render_3d_phase)
      .each([](const CameraComponent &camera_component) {
        const auto camera = camera_component.camera;
        BeginMode3D(camera);
      });

  world.system("End Render3D")
      .kind(on_post_render_3d_phase)
      .run([](flecs::iter &it) { EndMode3D(); });

  world.system("Draw FPS").kind(on_render_2d_phase).run([](flecs::iter &it) {
    DrawFPS(20, 20);
  });

  world.system("End Render").kind(on_post_render_phase).run([](flecs::iter &it) {
    EndDrawing();
  });

  world
      .system<const RenderableComponent, const ModelComponent,
              const MatrixComponent>("Render Models")
      .kind(on_render_3d_phase)
      .each([](const RenderableComponent &renderable, const ModelComponent &model_component,
               const MatrixComponent matrix_component) {
        DrawModel(model_component.model, GetPositionFromMatrix(matrix_component.matrix), 1.0f, WHITE);
      });

  world
      .system<const RenderableComponent, const SpherePrimitiveComponent,
              const MatrixComponent>("Draw Spheres")
      .kind(on_render_3d_phase)
      .each([](const RenderableComponent &renderable,
               const SpherePrimitiveComponent &sphere,
               const MatrixComponent matrix_component) {
        constexpr float kSphereRadius = 0.5f;
        DrawSphere(GetPositionFromMatrix(matrix_component.matrix), kSphereRadius, RED);
      });

  world
      .system<const RenderableComponent, const CapsulePrimitiveComponent,
              const MatrixComponent>("Draw Capsules")
      .kind(on_render_3d_phase)
      .each([](const RenderableComponent &renderable,
               const CapsulePrimitiveComponent &capsule,
               const MatrixComponent matrix_component) {
        constexpr float kCapsuleHeight = 2.0f;
        constexpr float kCapsuleRadius = 0.5f;
        constexpr int kCapsuleRings = 8;
        constexpr int kCapsuleSlices = 8;
        auto start_position = GetPositionFromMatrix(matrix_component.matrix);
        start_position.y -= kCapsuleHeight / 2.0f;
        auto end_position = start_position;
        end_position.y += kCapsuleHeight;
        DrawCapsule(start_position, end_position, kCapsuleRadius, kCapsuleRings, kCapsuleSlices, RED);
      });

  world
      .system<const RenderableComponent, const GridPrimitiveComponent>(
          "Draw Grid")
      .kind(on_render_3d_phase)
      .each([](const RenderableComponent &renderable, const GridPrimitiveComponent &grid) {
        DrawGrid(grid.slices, grid.spacing);
      });

  world.system<CameraComponent, const MatrixComponent>()
      .kind(on_begin_phase)
      .each([](CameraComponent &camera_component,
               const MatrixComponent &matrix_component) {
        return;
        camera_component.camera.position =
            GetPositionFromMatrix(matrix_component.matrix);
        camera_component.camera.up = GetUpVector(matrix_component.matrix);
        camera_component.camera.target =
            GetTargetForCamera(matrix_component.matrix);
      });
}

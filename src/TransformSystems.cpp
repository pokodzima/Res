#include "TransformSystems.h"
#include "Phases.h"
#include "RenderComponents.h"
#include "TransformComponents.h"
#include "raylib.h"

#include <flecs.h>

res::TransformSystems::TransformSystems(flecs::world &world) {
  world.module<TransformSystems>();

  auto on_pre_render_phase = world.lookup(kPreRenderPhaseName.data());

  world
      .system<const DebugCameraMovementComponent, CameraComponent>(
          "Debug Camera Movement")
      .kind(on_pre_render_phase)
      .each([](const DebugCameraMovementComponent &debug_movement,
               CameraComponent &camera_component) {
        UpdateCamera(&camera_component.camera, debug_movement.movement_type);
      });
}

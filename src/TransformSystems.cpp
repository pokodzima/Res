#include "TransformSystems.h"
#include "Phases.h"
#include "RenderComponents.h"
#include "TransformComponents.h"
#include "raylib.h"

#include <flecs.h>

res::TransformSystems::TransformSystems(flecs::world &world) {
  world.module<TransformSystems>();

  auto OnPreRenderPhase = world.lookup(OnPreRenderPhaseName.data());

  world
      .system<const DebugCameraMovementComponent, CameraComponent>(
          "Debug Camera Movement")
      .kind(OnPreRenderPhase)
      .each([](const DebugCameraMovementComponent &dcmc,
               CameraComponent &camera) {
        UpdateCamera(&camera.raylibCamera, dcmc.debug_movement_type);
      });
}

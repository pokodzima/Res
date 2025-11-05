#pragma once

#include <raylib.h>

namespace res {
struct RenderableComponent {};

struct ModelComponent {
  Model model;
};

struct CameraComponent {
  Camera3D camera;
};

struct SpherePrimitiveComponent {};

struct CapsulePrimitiveComponent {};

struct CubePrimitiveComponent {};

struct GridPrimitiveComponent {
  int slices;
  float spacing;
};
} // namespace res

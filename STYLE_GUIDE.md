# C++ Style Guide

This project follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with the following key conventions:

## Naming Conventions

### Variables
- Local variables: `snake_case`
  ```cpp
  auto entity_position = GetPositionFromMatrix(matrix);
  int vertex_count = 100;
  ```

- Function parameters: `snake_case`
  ```cpp
  void ProcessInput(float delta_time, bool is_active);
  ```

- Struct/class members: `snake_case` (for public data members in plain structs)
  ```cpp
  struct Position2dComponent {
      float x{0.0f};
      float y{0.0f};
  };
  ```

### Constants
- Compile-time constants: `kConstantName` (with 'k' prefix)
  ```cpp
  constexpr float kSphereRadius = 0.5f;
  constexpr int kMaxBodies = 65536;
  ```

### Types
- Classes, structs, enums: `PascalCase`
  ```cpp
  class Window;
  struct CameraComponent;
  enum class MotionType;
  ```

- Namespaces: `snake_case`
  ```cpp
  namespace res {
  namespace physics_utils {
  ```

### Functions
- Functions and methods: `PascalCase` (following raylib convention for consistency)
  ```cpp
  Vector3 GetPositionFromMatrix(const Matrix& matrix);
  void UpdateCamera(Camera3D* camera, int mode);
  ```

## Comments
- Use `//` for single-line comments
- Place comments above the code they describe
- Keep comments concise and meaningful

## General Guidelines
- Use `constexpr` for compile-time constants
- Use `auto` where type is obvious from context
- Prefer `nullptr` over `NULL` or `0`
- Use `[[nodiscard]]` for functions whose return value should not be ignored
- Avoid magic numbers; use named constants instead

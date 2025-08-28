# Res Game Engine

A small, experimental C++20 game engine prototype built around an ECS architecture. Rendering is powered by raylib, gameplay data is modeled with flecs, physics uses Jolt Physics, and in‑game/editor UI is provided by Dear ImGui via rlImGui.

## Features
- C++20, CMake build with automatic dependency fetching (FetchContent)
- ECS with flecs (components + systems layout under src/)
- Rendering and window/input via raylib
- Rigid body physics via Jolt Physics
- Immediate‑mode UI with Dear ImGui (integrated through rlImGui)
- Basic transform hierarchy, render, physics, and debug systems

## Tech stack
- raylib 5.5
- Jolt Physics v5.3.0
- flecs v4.1.1
- Dear ImGui v1.92.2 + rlImGui

All third‑party libraries are fetched and built automatically by CMake; no manual installs are required beyond standard toolchains and platform SDKs.

## Acknowledgements
- raysan5/raylib
- jrouwe/JoltPhysics
- SanderMertens/flecs
- ocornut/imgui and raylib-extras/rlImGui


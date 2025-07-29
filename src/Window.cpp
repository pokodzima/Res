#include "Window.h"

#include "raylib.h"
#include "rlImGui.h"

res::Window::Window(const int width, const int height, const int FPS, const std::string& name)
{
    InitWindow(width, height, name.c_str());
    SetTargetFPS(FPS);

    rlImGuiSetup(true);
}

res::Window::~Window()
{
    rlImGuiShutdown();
    CloseWindow();
}

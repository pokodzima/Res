#pragma once
#include <string>

namespace res
{
    class Window
    {
    public:
        Window(int width, int height, int FPS, const std::string& name);

        ~Window();
    };
}

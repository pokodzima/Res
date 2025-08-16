#pragma once
#include "flecs.h"
#include "Window.h"

namespace res
{
    class App
    {
    public:
        App();
        void Run();
    private:
        flecs::world mWorld;
        Window mWindow;
    };
}

#include "App.h"

int main()
{
    const auto app = new res::App();
    app->Run();
    delete app;
    return 0;
}

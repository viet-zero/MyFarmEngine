#include "Core/Application.h"

int main()
{
    MyEngine::Application app;

    if (!app.Initialize())
    {
        return -1;
    }

    app.Run();

    app.Shutdown();

    return 0;
}
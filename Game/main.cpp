#include "Core/Application.h"
#include "Game/GameApp/Game.h"
#include "Game/GameApp/GameConfig.h"
#include <iostream>
#include <string_view>

int main(int argc, char** argv)
{
    unsigned int maxFrames = 0;
    MyGame::GameConfig config;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view argument(argv[i]);
        if (argument == "--smoke-test")
            maxFrames = 5;
        else if (argument == "--no-vsync")
            config.vsync = false;
        else
        {
            std::cerr << "Usage: MyFarmEngine [--smoke-test] [--no-vsync]\n";
            return 2;
        }
    }
    MyGame::Game game;
    MyEngine::Application app(game);
    if (!app.Initialize(config.windowWidth, config.windowHeight,
                        config.title, config.vsync))
    {
        std::cerr << "Application initialization failed.\n";
        return 1;
    }
    app.Run(maxFrames);
    app.Shutdown();
    return 0;
}

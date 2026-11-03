#pragma once

namespace MyGame
{
    struct GameConfig
    {
        unsigned int windowWidth = 1280;
        unsigned int windowHeight = 720;
        const char* title = "My Farm - Cozy Morning";
        bool vsync = true;
    };
}

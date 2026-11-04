#pragma once
namespace MyGame
{
    struct SceneOptions
    {
        bool demoMode=false;
        bool npcAI=false;       // Enables only the opt-in demo patrol, not a production AI system.
        bool petFollow=false;
        bool debugOverlay=false;
    };
}

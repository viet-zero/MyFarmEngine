#pragma once
#include "Engine/Event/EventBus.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace MyGame
{
    enum class TargetKind {Object,Crop,Pickup,FruitTree};
    struct InteractionTarget
    {
        std::string id;
        TargetKind kind=TargetKind::Object;
        glm::vec2 position{0,0};
        int health=2;
        bool active=true,planted=false,ready=false;
        float age=0;
        int stock=0,maxStock=0;
        float regrowSeconds=30.0f;
    };
    struct InteractionActor
    {
        glm::vec2 position{0,0},facing{0,1};
        float stamina=20,cooldown=0;
        int inventoryCount=3,inventoryCapacity=12;
        bool busy=false,uiCapturesInput=false;
    };
    enum class InteractionResult {Success,NoTarget,Inactive,TooFar,WrongFacing,Busy,Cooldown,
                                  NoStamina,InventoryFull,NoSeeds,WrongTool,NotReady};
    class InteractionSystem
    {
    public:
        static constexpr float Range=100.0f,CooldownSeconds=0.35f,StaminaCost=2.0f;
        static InteractionTarget* Nearest(const InteractionActor& actor,std::vector<InteractionTarget>& targets);
        static InteractionResult Use(InteractionActor& actor,InteractionTarget* target,int selectedItem,EventBus& events);
        static void Tick(InteractionActor& actor,std::vector<InteractionTarget>& targets,float deltaTime);
    };
}

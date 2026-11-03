#pragma once
#include <glm/glm.hpp>
#include <cstddef>
#include <string>
#include <vector>

namespace MyGame
{
    enum class GameEventKind {ToolUsed,ObjectHit,CropPlanted,CropHarvested,ItemPicked};
    struct GameEvent
    {
        GameEventKind kind;
        std::string targetId;
        glm::vec2 position{0,0};
        int amount=1;
    };
    class EventBus
    {
    public:
        static constexpr std::size_t MaxPending=256;
        EventBus(){m_Events.reserve(MaxPending);}
        bool Publish(GameEvent event);
        std::vector<GameEvent> Take();
        std::size_t Pending() const {return m_Events.size();}
        void Clear(){m_Events.clear();}
    private:
        std::vector<GameEvent> m_Events;
    };
}

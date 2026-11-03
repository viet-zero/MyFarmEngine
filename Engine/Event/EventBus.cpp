#include "EventBus.h"
#include <cmath>

namespace MyGame
{
    bool EventBus::Publish(GameEvent event)
    {
        if(m_Events.size()>=MaxPending || !std::isfinite(event.position.x) ||
           !std::isfinite(event.position.y) || event.amount<=0) return false;
        m_Events.push_back(std::move(event));return true;
    }
    std::vector<GameEvent> EventBus::Take()
    {
        std::vector<GameEvent> result;result.swap(m_Events);m_Events.reserve(MaxPending);
        return result;
    }
}

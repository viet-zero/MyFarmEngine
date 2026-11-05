#include "EffectPool.h"
#include <algorithm>
#include <cmath>

namespace MyGame
{
    bool EffectPool::Spawn(Effect effect)
    {
        if(m_Active.size()>=Capacity || !std::isfinite(effect.position.x) ||
           !std::isfinite(effect.position.y) || !std::isfinite(effect.velocity.x) ||
           !std::isfinite(effect.velocity.y) || !std::isfinite(effect.lifetime) ||
           effect.lifetime<=0 || effect.lifetime>10) return false;
        effect.age=0;m_Active.push_back(effect);return true;
    }
    void EffectPool::Consume(std::span<const GameEvent> events)
    {
        for(const auto& event:events)
        {
            Spawn({EffectKind::Particle,event.position,{-18,-28},0,0.45f,event.amount});
            Spawn({EffectKind::Particle,event.position,{18,-38},0,0.55f,event.amount});
            if(event.kind==GameEventKind::ItemPicked || event.kind==GameEventKind::CropHarvested ||
               event.kind==GameEventKind::FruitHarvested)
                Spawn({EffectKind::FloatingText,event.position,{0,-34},0,1.1f,event.amount});
            if(event.kind==GameEventKind::ObjectHit)
                m_Shake=std::min(4.0f,m_Shake+1.5f);
        }
    }
    void EffectPool::Update(float deltaTime)
    {
        if(!std::isfinite(deltaTime) || deltaTime<=0) return;
        const float dt=std::min(deltaTime,0.1f);
        for(auto& effect:m_Active)
        {
            effect.age+=dt;effect.position+=effect.velocity*dt;
        }
        std::erase_if(m_Active,[](const Effect& effect){return effect.age>=effect.lifetime;});
        m_Shake=std::max(0.0f,m_Shake-12.0f*dt);
    }
    void EffectPool::Clear(){m_Active.clear();m_Shake=0;}
}

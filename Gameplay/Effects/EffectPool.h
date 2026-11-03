#pragma once
#include "Engine/Event/EventBus.h"
#include <glm/glm.hpp>
#include <cstddef>
#include <vector>

namespace MyGame
{
    enum class EffectKind {Particle,FloatingText};
    struct Effect
    {
        EffectKind kind;
        glm::vec2 position{0,0},velocity{0,0};
        float age=0,lifetime=1;
        int value=1;
    };
    class EffectPool
    {
    public:
        static constexpr std::size_t Capacity=128;
        EffectPool(){m_Active.reserve(Capacity);}
        bool Spawn(Effect effect);
        void Consume(const std::vector<GameEvent>& events);
        void Update(float deltaTime);
        void Clear();
        const std::vector<Effect>& Active() const {return m_Active;}
        float Shake() const {return m_Shake;}
    private:
        std::vector<Effect> m_Active;
        float m_Shake=0;
    };
}

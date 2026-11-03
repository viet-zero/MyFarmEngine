#pragma once
#include <glm/glm.hpp>
#include <cstddef>
#include <vector>

namespace MyGame
{
    enum class RenderKind {Crop,Pickup,Actor,Animal,TreeCanopy,WorldObject};
    struct RenderEntry
    {
        RenderKind kind;
        std::size_t index;
        float footY;
        std::size_t sequence;
    };

    // Reused every frame: cull first, then sort only visible world sprites by foot Y.
    class RenderOrder
    {
    public:
        void Begin(glm::vec2 viewOrigin,glm::vec2 viewSize);
        bool Submit(RenderKind kind,std::size_t index,glm::vec2 origin,glm::vec2 size,float footY);
        void Sort();
        const std::vector<RenderEntry>& Entries() const {return m_Entries;}
    private:
        glm::vec2 m_ViewOrigin{},m_ViewSize{};
        std::vector<RenderEntry> m_Entries;
    };
}

#pragma once
#include <glm/glm.hpp>
#include <algorithm>
namespace MyGame
{
    struct SceneInput
    {
        glm::vec2 movement{0,0};
        bool running=false,interact=false,action=false,hurt=false,reset=false,switchCharacter=false;
        int teleport=0,zoom=0,selectedItem=-1;
        bool cancel=false,paused=false,toggleStorage=false,toggleMarket=false,save=false,load=false,toggleTileGrid=false,nextArtSeason=false;
    };
    // Held state is sampled once per frame. Edge commands survive zero-tick frames,
    // then are consumed only by the first fixed tick; no raw GLFW polling in a catch-up loop.
    class SceneInputBuffer
    {
    public:
        void Capture(const SceneInput& input)
        {
            if(input.paused){Clear();m_Pending.paused=true;return;}
            m_Pending.paused=false;m_Pending.movement=input.movement;m_Pending.running=input.running;
            m_Pending.interact|=input.interact;m_Pending.action|=input.action;m_Pending.hurt|=input.hurt;
            m_Pending.reset|=input.reset;m_Pending.switchCharacter|=input.switchCharacter;m_Pending.cancel|=input.cancel;
            m_Pending.toggleStorage|=input.toggleStorage;
            m_Pending.toggleMarket|=input.toggleMarket;
            m_Pending.save|=input.save;
            m_Pending.load|=input.load;
            m_Pending.toggleTileGrid|=input.toggleTileGrid;
            m_Pending.nextArtSeason|=input.nextArtSeason;
            if(input.teleport) m_Pending.teleport=input.teleport;
            if(input.selectedItem>=0) m_Pending.selectedItem=input.selectedItem;
            m_Pending.zoom=std::clamp(m_Pending.zoom+std::clamp(input.zoom,-8,8),-8,8);
        }
        SceneInput Consume()
        {
            const auto result=m_Pending;
            m_Pending={};m_Pending.movement=result.movement;m_Pending.running=result.running;m_Pending.paused=result.paused;
            return result;
        }
        void Clear(){m_Pending={};}
    private:
        SceneInput m_Pending;
    };
}

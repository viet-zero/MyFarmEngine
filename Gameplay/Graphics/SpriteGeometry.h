#pragma once
#include "Gameplay/Assets/AssetManifest.h"
#include <glm/glm.hpp>
#include <vector>

namespace MyGame
{
    struct SpriteFrameBounds
    {
        int left=0,top=0,width=0,height=0;
        int frameWidth=0,frameHeight=0,referenceHeight=0;
    };
    struct SpritePlacement
    {
        glm::vec2 origin{0,0},size{0,0};
        glm::vec2 visibleOrigin{0,0},visibleSize{0,0};
    };
    // Alpha-derived bounds are presentation metadata, never gameplay collision or art approval.
    std::vector<SpriteFrameBounds> MeasureSpriteFrames(const MyEngine::ImageData& image,const AssetRecord& record);
    SpritePlacement PlaceSprite(const SpriteFrameBounds& frame,glm::vec2 feet,float height,bool flipX=false);
    SpritePlacement FitSprite(const SpriteFrameBounds& frame,glm::vec2 origin,glm::vec2 box);
    struct HotbarLayout
    {
        float slot=0,gap=0,padding=0,x=0,y=0,width=0,height=0;
        static HotbarLayout ForViewport(int width,int height);
    };
    inline constexpr float HumanHeight=132.0f,PetHeight=72.0f;
}

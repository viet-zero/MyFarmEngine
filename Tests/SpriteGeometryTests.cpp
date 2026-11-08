#include "Gameplay/Graphics/SpriteGeometry.h"
#include "Gameplay/Graphics/WorldArt.h"
#include <set>
#include <fstream>
#include <limits>
#include <cmath>
#include <iostream>
namespace { int failures=0;void Check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';++failures;}}
bool Near(float a,float b){return std::abs(a-b)<0.001f;} }
int main()
{
    using namespace MyGame;
    std::set<unsigned int> masks;
    for(unsigned int mask=0;mask<256;++mask)
    {
        const auto canonical=CanonicalTerrainMask(mask);
        masks.insert(canonical);
        Check(CanonicalTerrainMask(canonical)==canonical,"terrain mask is not idempotent");
        Check((canonical&2)==0 || (canonical&5)==5,"orphan northeast shore corner");
        Check((canonical&8)==0 || (canonical&20)==20,"orphan southeast shore corner");
        Check((canonical&32)==0 || (canonical&80)==80,"orphan southwest shore corner");
        Check((canonical&128)==0 || (canonical&65)==65,"orphan northwest shore corner");
    }
    Check(masks.size()==47,"shore mask set must contain 47 variants");
    const glm::vec2 directions[]={{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}};
    for(int i=0;i<8;++i) Check(ArtDirection(directions[i],2)==i,"eight-way art direction mismatch");
    Check(ArtDirection({0,0},5)==5,"idle changed facing");
    Check(ArtDirection({std::numeric_limits<float>::quiet_NaN(),0},3)==3,"invalid input changed facing");
    const SpriteFrameBounds authored{20,20,120,140,192,192,140};
    const auto anchored=PlaceSprite(authored,{300,400},140,false,glm::vec2{96,180});
    Check(Near(anchored.origin.x+96,300) && Near(anchored.origin.y+180,400),"authored root moved off ground");
    const auto mirrored=PlaceSprite(authored,{300,400},140,true,glm::vec2{80,180});
    Check(Near(mirrored.origin.x+112,300),"mirrored authored root incorrect");
    MyEngine::ImageData image;image.width=20;image.height=10;image.rgba.resize(20*10*4);
    for(int y=2;y<9;++y) for(int x=2;x<7;++x) image.rgba[(y*20+x)*4+3]=255;
    for(int y=1;y<8;++y) for(int x=13;x<17;++x) image.rgba[(y*20+x)*4+3]=255;
    image.rgba[3]=1; // Invisible fringe must not move the foot pivot.
    AssetRecord record;record.columns=2;record.rows=1;record.frameWidth=10;record.frameHeight=10;record.frameCount=2;
    const auto frames=MeasureSpriteFrames(image,record);
    Check(frames.size()==2,"two atlas frames not measured");if(frames.size()!=2) return 1;
    Check(frames[0].left==2 && frames[0].top==2 && frames[0].width==5 && frames[0].height==7,"alpha bounds incorrect");
    Check(frames[1].left==3 && frames[1].top==1 && frames[1].referenceHeight==7,"frame isolation/reference scale incorrect");
    for(const bool flip:{false,true}) for(const auto& frame:frames)
    {
        const auto p=PlaceSprite(frame,{100,200},HumanHeight,flip);
        Check(Near(p.visibleOrigin.y+p.visibleSize.y,200),"visible feet left ground anchor");
        Check(Near(p.visibleOrigin.x+p.visibleSize.x*0.5f,100),"mirrored pivot left foot center");
        Check(Near(p.size.x/p.size.y,1),"source aspect ratio distorted");
    }
    const auto fit=FitSprite(frames[0],{10,20},{32,24});
    Check(fit.visibleSize.x<=32 && fit.visibleSize.y<=24 && Near(fit.visibleSize.x/fit.visibleSize.y,5.0f/7),"icon fit distorted or escaped box");
    for(const auto view:{glm::ivec2{1280,720},glm::ivec2{640,360},glm::ivec2{200,100}})
    {
        const auto bar=HotbarLayout::ForViewport(view.x,view.y);
        Check(bar.x>=0 && bar.y>=0 && bar.x+bar.width<=view.x && bar.y+bar.height<=view.y,"hotbar outside viewport");
        Check(Near(bar.width,4*bar.slot+3*bar.gap+2*bar.padding),"toolbar spacing inconsistent");
    }
    record.frameCount=3;Check(MeasureSpriteFrames(image,record).empty(),"out-of-grid frame accepted");
    const auto badScale=std::filesystem::current_path()/"invalid_visual_scale.csv";
    {std::ofstream output(badScale);output<<"key,value\nhuman,160\ntree,-1\n";}
    VisualScale scale;
    Check(!scale.Load(badScale) && Near(scale.human,112),"invalid scale partially changed configuration");
    std::filesystem::remove(badScale);
    return failures?1:0;
}

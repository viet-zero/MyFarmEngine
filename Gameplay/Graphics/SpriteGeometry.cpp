#include "SpriteGeometry.h"
#include <algorithm>
#include <cmath>

namespace MyGame
{
    std::vector<SpriteFrameBounds> MeasureSpriteFrames(const MyEngine::ImageData& image,const AssetRecord& record)
    {
        std::vector<SpriteFrameBounds> result;
        if(image.width<=0 || image.height<=0 || record.frameCount<=0 ||
           image.rgba.size()!=static_cast<std::size_t>(image.width)*image.height*4) return result;
        int reference=0;
        for(int i=0;i<record.frameCount;++i)
        {
            if(!AssetManifest::FrameUv(record,i)) return {};
            const int index=record.firstFrame+i;
            const int ox=record.marginX+(index%record.columns)*(record.frameWidth+record.spacingX);
            const int oy=record.marginY+(index/record.columns)*(record.frameHeight+record.spacingY);
            if(ox<0 || oy<0 || ox+record.frameWidth>image.width || oy+record.frameHeight>image.height) return {};
            int left=record.frameWidth,top=record.frameHeight,right=-1,bottom=-1;
            for(int y=0;y<record.frameHeight;++y)
                for(int x=0;x<record.frameWidth;++x)
                {
                    const auto pixel=(static_cast<std::size_t>(oy+y)*image.width+ox+x)*4;
                    if(image.rgba[pixel+3]<32) continue;
                    left=std::min(left,x);top=std::min(top,y);right=std::max(right,x);bottom=std::max(bottom,y);
                }
            SpriteFrameBounds frame;
            frame.frameWidth=record.frameWidth;frame.frameHeight=record.frameHeight;
            if(right>=left && bottom>=top)
            {frame.left=left;frame.top=top;frame.width=right-left+1;frame.height=bottom-top+1;}
            reference=std::max(reference,frame.height);result.push_back(frame);
        }
        for(auto& frame:result) frame.referenceHeight=reference;
        return result;
    }
    SpritePlacement PlaceSprite(const SpriteFrameBounds& frame,glm::vec2 feet,float height,bool flipX,std::optional<glm::vec2> authoredPivot)
    {
        if(frame.width<=0 || frame.height<=0 || frame.referenceHeight<=0 ||
           !std::isfinite(height) || height<=0) return {};
        const float scale=height/static_cast<float>(frame.referenceHeight);
        const float contentX=static_cast<float>(flipX?frame.frameWidth-frame.left-frame.width:frame.left);
        glm::vec2 pivot{contentX+frame.width*0.5f,static_cast<float>(frame.top+frame.height)};
        if(authoredPivot)
        {
            if(!std::isfinite(authoredPivot->x) || !std::isfinite(authoredPivot->y)) return {};
            pivot=*authoredPivot;
            if(flipX) pivot.x=frame.frameWidth-pivot.x;
        }
        SpritePlacement result;
        result.origin=feet-pivot*scale;
        result.size=glm::vec2(frame.frameWidth,frame.frameHeight)*scale;
        result.visibleOrigin=result.origin+glm::vec2(contentX,frame.top)*scale;
        result.visibleSize=glm::vec2(frame.width,frame.height)*scale;
        return result;
    }
    SpritePlacement FitSprite(const SpriteFrameBounds& frame,glm::vec2 origin,glm::vec2 box)
    {
        if(frame.width<=0 || frame.height<=0 || box.x<=0 || box.y<=0) return {};
        const float scale=std::min(box.x/frame.width,box.y/frame.height);
        SpritePlacement result;
        result.visibleSize=glm::vec2(frame.width,frame.height)*scale;
        result.visibleOrigin=origin+(box-result.visibleSize)*0.5f;
        result.origin=result.visibleOrigin-glm::vec2(frame.left,frame.top)*scale;
        result.size=glm::vec2(frame.frameWidth,frame.frameHeight)*scale;
        return result;
    }
    HotbarLayout HotbarLayout::ForViewport(int width,int height)
    {
        if(width<=0 || height<=0) return {};
        const float scale=std::min({1.0f,static_cast<float>(width)/320.0f,static_cast<float>(height)/144.0f});
        HotbarLayout bar;
        bar.slot=64*scale;bar.gap=8*scale;bar.padding=12*scale;
        bar.width=4*bar.slot+3*bar.gap+2*bar.padding;bar.height=bar.slot+2*bar.padding;
        bar.x=(width-bar.width)*0.5f;bar.y=height-bar.height-16*scale;
        return bar;
    }
}

#include "Graphics/Camera2D.h"
#include "Gameplay/Player/Movement2D.h"
#include <cmath>
#include <iostream>

namespace
{
    int failures = 0;
    void Check(bool condition, const char* label)
    {
        if (!condition) { std::cerr << label << '\n'; ++failures; }
    }
    bool Near(glm::vec2 a, glm::vec2 b) { return glm::length(a-b)<0.01f; }
}

int main()
{
    MyEngine::Camera2D camera;
    camera.SetWorldSize({4000,3000});
    camera.Resize(1280,720);
    camera.SetPosition({2000,1500});
    Check(Near(camera.WorldToScreen(camera.Position()),{640,360}), "camera center is not screen center");
    const glm::vec2 point{2111,1444};
    Check(Near(camera.ScreenToWorld(camera.WorldToScreen(point)),point), "coordinate round trip failed");
    const auto clip = camera.ViewProjection()*glm::vec4(camera.Position(),0,1);
    Check(std::abs(clip.x)<0.001f && std::abs(clip.y)<0.001f, "projection does not map center to origin");
    const auto topLeft = camera.ViewProjection()*glm::vec4(camera.Position()-camera.ViewSize()*0.5f,0,1);
    Check(std::abs(topLeft.x+1)<0.001f && std::abs(topLeft.y-1)<0.001f, "projection orientation mismatch");
    camera.SetZoom(2);
    Check(Near(camera.ViewSize(),{640,360}), "zoom did not halve the visible world");
    camera.Resize(720,1280);
    Check(std::abs(camera.ViewSize().x/camera.ViewSize().y-720.0f/1280)<0.001f,
          "portrait resize distorted projection");
    camera.SetPosition({-100,99999});
    const auto half=camera.ViewSize()*0.5f;
    Check(Near(camera.Position(),{half.x,3000-half.y}), "camera world constraints failed");
    camera.SetWorldSize({1600,1067});
    camera.Resize(2200,500);
    camera.SetZoom(0.5f);
    Check(camera.ViewSize().x<=1600.1f && camera.ViewSize().y<=1067.1f, "zoom out exposed world border");
    camera.Resize(0,0);
    Check(std::isfinite(camera.ViewProjection()[0][0]), "zero viewport generated invalid matrix");
    camera.SetWorldSize({4000,3000});
    camera.Resize(1280,720);
    camera.SetZoom(1);
    camera.SetPosition({2000,1500});
    auto split=camera;
    camera.Follow({2200,1600},0.1f);
    for(int i=0;i<10;++i) split.Follow({2200,1600},0.01f);
    Check(Near(camera.Position(),split.Position()), "camera follow depends on timestep split");

    const glm::vec2 start{800,600}, world{4000,3000};
    const auto straight=MyGame::MoveWithinBounds(start,{1,0},180,1,world);
    const auto diagonal=MyGame::MoveWithinBounds(start,{1,1},180,1,world);
    Check(std::abs(glm::length(straight-start)-glm::length(diagonal-start))<0.001f,
          "diagonal movement is faster");
    auto sixty=start, thirty=start;
    for(int i=0;i<60;++i) sixty=MyGame::MoveWithinBounds(sixty,{1,0},180,1.0f/60,world);
    for(int i=0;i<30;++i) thirty=MyGame::MoveWithinBounds(thirty,{1,0},180,1.0f/30,world);
    Check(Near(sixty,thirty), "movement distance depends on frame rate");
    Check(Near(MyGame::MoveWithinBounds(start,{},180,1,world),start), "idle player moved");
    Check(Near(MyGame::MoveWithinBounds(start,{1,0},180,-1,world),start), "negative time moved player");
    Check(Near(MyGame::MoveWithinBounds({74,74},{-1,-1},999,1,world),{72,72}), "player escaped world bounds");
    return failures==0?0:1;
}

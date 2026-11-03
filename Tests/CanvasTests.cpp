#include "Core/Window.h"
#include "Graphics/Canvas2D.h"
#include "Resources/AssetPath.h"
#include "Gameplay/Graphics/AssetTextures.h"
#include <glad/gl.h>
#include <stb_image.h>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

namespace
{
    int failures = 0;
    void Check(bool value, const char* message)
    {
        if (!value) { std::cerr << message << '\n'; ++failures; }
    }
    std::array<unsigned char, 4> Read(int x, int y)
    {
        std::array<unsigned char, 4> pixel{};
        glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
        return pixel;
    }
}

int main()
{
    MyEngine::Window window;
    if (!window.Create(320, 240, "Canvas pixel regression", false)) return 1;
    MyEngine::Canvas2D canvas;
    MyEngine::Texture2D texture;
    if (!canvas.Initialize() || !texture.Load(MyEngine::FindAsset("Backgrounds/cozy_farm_morning.png")))
        return 1;
    const auto path = MyEngine::FindAsset("Backgrounds/cozy_farm_morning.png");
    std::ifstream file(path, std::ios::binary);
    const std::vector<unsigned char> encoded{std::istreambuf_iterator<char>(file), {}};
    int width = 0, height = 0, channels = 0;
    auto* expected = stbi_load_from_memory(encoded.data(), static_cast<int>(encoded.size()),
                                           &width, &height, &channels, 4);
    if (!expected) return 1;

    unsigned int framebuffer = 0, target = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glGenTextures(1, &target);
    glBindTexture(GL_TEXTURE_2D, target);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target, 0);
    Check(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "incomplete test framebuffer");
    glViewport(0, 0, width, height);
    canvas.Begin(width, height);
    canvas.Image(texture, 0, 0, static_cast<float>(width), static_cast<float>(height));
    canvas.End();
    // Full-size image pixels must preserve PNG orientation and RGB values.
    for (const int y : {height/8, height/2, height*7/8})
        for (const int x : {width/8, width/2, width*7/8})
        {
            const auto actual = Read(x, height-1-y);
            for (int c = 0; c < 4; ++c)
                Check(std::abs(static_cast<int>(actual[c]) - expected[(y*width+x)*4+c]) <= 2,
                      "PNG orientation or color mismatch");
        }
    stbi_image_free(expected);

    // Cover must fill wide and tall windows without leaving unpainted edges.
    for (const auto dimensions : {std::array{500,200}, std::array{200,500}})
    {
        glViewport(0, 0, dimensions[0], dimensions[1]);
        glClearColor(1, 0, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        canvas.Begin(dimensions[0], dimensions[1]);
        canvas.Cover(texture);
        canvas.End();
        for (const int x : {0, dimensions[0]-1})
            for (const int y : {0, dimensions[1]-1})
                Check(Read(x,y)[3] == 255, "cover left an unpainted corner");
    }
    glViewport(0, 0, 32, 32);
    canvas.Begin(32, 32);
    canvas.Rectangle(0, 0, 32, 32, {0,0,1,1});
    canvas.Rectangle(0, 0, 32, 32, {1,0,0,0.5f});
    canvas.End();
    const auto blended = Read(16,16);
    Check(std::abs(static_cast<int>(blended[0])-128) <= 1 &&
          std::abs(static_cast<int>(blended[2])-127) <= 1, "alpha blend mismatch");
    glClearColor(0,0,1,1);glClear(GL_COLOR_BUFFER_BIT);
    canvas.Begin(32,32);
    canvas.SpriteEllipse(texture,0,0,32,32,{0,0,1,1});
    canvas.End();
    Check(Read(0,0)[2]==255 && Read(16,16)[2]!=255,"ellipse mask failed at corner/center");
    canvas.Begin(32,32);
    canvas.Rectangle(0,0,2,2,{1,0,0,1});
    canvas.End();
    Check(Read(0,31)[0]==255,"ellipse state leaked into next pass");

    // The world pass uses the actual camera matrix rather than screen coordinates.
    MyEngine::Camera2D camera;
    camera.SetWorldSize({4000,3000});
    camera.Resize(64,64);
    camera.SetPosition({2000,1500});
    glViewport(0,0,64,64);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    canvas.BeginWorld(64,64,camera);
    canvas.Rectangle(1910,1410,180,180,{0,1,0,1});
    canvas.End();
    Check(Read(32,32)[1]==255 && Read(20,20)[1]==0, "world projection misplaced rectangle");
    camera.SetZoom(2);
    glClear(GL_COLOR_BUFFER_BIT);
    canvas.BeginWorld(64,64,camera);
    canvas.Rectangle(1910,1410,180,180,{0,1,0,1});
    canvas.End();
    Check(Read(20,20)[1]==255, "camera zoom did not enlarge world geometry");

    MyEngine::Texture2D atlas;
    const auto atlasPath=MyEngine::FindAsset("Art/Items/farm_items_atlas.png");
    Check(atlas.Load(atlasPath), "new item atlas failed to load");
    std::ifstream atlasFile(atlasPath,std::ios::binary);
    const std::vector<unsigned char> atlasEncoded{std::istreambuf_iterator<char>(atlasFile),{}};
    int aw=0, ah=0;
    auto* atlasPixels=stbi_load_from_memory(atlasEncoded.data(),static_cast<int>(atlasEncoded.size()),&aw,&ah,&channels,4);
    if (atlasPixels)
    {
        const int cw=aw/2, ch=ah/2;
        Check(cw<=width && ch<=height,"atlas cell exceeds test target");
        glViewport(0,0,cw,ch);
        for(int item=0;item<4;++item)
        {
            const int column=item%2, row=item/2;
            glClear(GL_COLOR_BUFFER_BIT);
            canvas.Begin(cw,ch);
            canvas.Sprite(atlas,0,0,static_cast<float>(cw),static_cast<float>(ch),
                          {column*0.5f,row*0.5f,0.5f,0.5f});
            canvas.End();
            bool checked=false;
            for(int y=ch/4;y<ch*3/4 && !checked;++y)
                for(int x=cw/4;x<cw*3/4 && !checked;++x)
                {
                    const auto index=((y+row*ch)*aw+x+column*cw)*4;
                    if(atlasPixels[index+3]<250) continue;
                    const auto pixel=Read(x,ch-1-y);
                    for(int c=0;c<3;++c)
                        Check(std::abs(static_cast<int>(pixel[c])-
                              static_cast<int>(std::lround(atlasPixels[index+c]*atlasPixels[index+3]/255.0)))<=3,
                              "atlas UV selected wrong item");
                    checked=true;
                }
            Check(checked,"atlas item has no opaque test sample");
        }
        stbi_image_free(atlasPixels);
    }
    else Check(false,"atlas PNG decoding failed");
    atlas.Reset();
    MyEngine::Shader shader;
    Check(shader.Load(MyEngine::FindAsset("Shaders/canvas.vert"),MyEngine::FindAsset("Shaders/canvas.frag")),
          "external GLSL shader failed");
    const auto program=shader.Id();
    Check(!shader.Load(MyEngine::FindAsset("Shaders/canvas.vert"),MyEngine::FindAsset("Shaders/missing.frag")) &&
          shader.Id()==program && glIsProgram(program), "failed shader replacement lost a valid program");
    shader.Reset();
    shader.Reset();
    MyGame::AssetTextures assets;
    Check(assets.Initialize(MyEngine::AssetRoot()),"runtime asset manifest did not load");
    const auto* farmer=assets.Load("player_linh");
    Check(farmer && farmer==assets.Load("player_linh") && assets.CachedCount()==1,
          "validated texture cache did not reuse an asset");
    Check(assets.Load("library_snhr_v11_characters_character_0001_sprite")==nullptr && assets.CachedCount()==1,
          "needs_review artwork was uploaded to GPU");
    Check(assets.Load("unknown_asset")==nullptr && assets.CachedCount()==1,"unknown asset activated");
    const auto farmerId=farmer?farmer->Id():0;
    assets.Shutdown();
    Check(farmerId && !glIsTexture(farmerId),"cached texture leaked");
    const auto original = texture.Id();
    Check(!texture.Load(MyEngine::FindAsset("does-not-exist.png")) && texture.Id() == original,
          "failed PNG load destroyed an existing texture");
    texture.Reset();
    texture.Reset();
    Check(!glIsTexture(original), "texture resource was not deleted");
    canvas.Shutdown();
    canvas.Shutdown();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &target);
    Check(glGetError() == GL_NO_ERROR, "OpenGL error in canvas tests");
    return failures == 0 ? 0 : 1;
}

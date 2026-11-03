#include "DemoScene2D.h"
#include "Gameplay/Player/Movement2D.h"
#include "Input/Input.h"
#include "Resources/AssetPath.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    const glm::vec2 WorldSize{1600, 1067};
    constexpr std::array<std::array<float,4>,4> ItemRegions = {{
        {0,0,0.5f,0.5f}, {0.5f,0,0.5f,0.5f},
        {0,0.5f,0.5f,0.5f}, {0.5f,0.5f,0.5f,0.5f}
    }};
    constexpr std::array<const char*,4> ItemNames = {"Watering can", "Garden hoe", "Seeds", "Strawberry cake"};
}

namespace MyGame
{
    bool DemoScene2D::Initialize()
    {
        m_Ready = m_Map.Load(MyEngine::FindAsset("Art/World/garden_world.png")) &&
            m_Characters[0].Load(MyEngine::FindAsset("Art/Characters/farmer_linh.png")) &&
            m_Characters[1].Load(MyEngine::FindAsset("Art/Characters/farmer_an.png")) &&
            m_Items.Load(MyEngine::FindAsset("Art/Items/farm_items_atlas.png"));
        Reset();
        return m_Ready;
    }
    void DemoScene2D::Reset()
    {
        m_Player = {760,620};
        m_WalkTime = 0;
        m_Moving = false;
        m_Camera.SetWorldSize(WorldSize);
        m_Camera.SetZoom(0.85f);
        m_Camera.SetPosition(m_Player);
    }
    void DemoScene2D::Update(float deltaTime)
    {
        glm::vec2 direction{
            (MyEngine::Input::IsKeyPressed(GLFW_KEY_D) ? 1.0f : 0.0f) -
            (MyEngine::Input::IsKeyPressed(GLFW_KEY_A) ? 1.0f : 0.0f),
            (MyEngine::Input::IsKeyPressed(GLFW_KEY_S) ? 1.0f : 0.0f) -
            (MyEngine::Input::IsKeyPressed(GLFW_KEY_W) ? 1.0f : 0.0f)};
        m_Moving = glm::length(direction) > 0;
        const float speed = MyEngine::Input::IsKeyPressed(GLFW_KEY_LEFT_SHIFT) ? 300.0f : 180.0f;
        m_Player = MoveWithinBounds(m_Player, direction, speed, deltaTime, WorldSize);
        if (m_Moving) m_WalkTime += deltaTime;
        else m_WalkTime = 0;
        if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_Q)) m_Camera.SetZoom(m_Camera.Zoom()/1.15f);
        if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_E)) m_Camera.SetZoom(m_Camera.Zoom()*1.15f);
        if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_R)) Reset();
        if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_TAB)) m_Character = 1-m_Character;
        for (int i=0; i<4; ++i)
            if (MyEngine::Input::ConsumeKeyPress(GLFW_KEY_1+i))
            {
                m_SelectedItem = i;
                if (auto* window = glfwGetCurrentContext())
                {
                    const std::string title = std::string("My Farm | ") + ItemNames[i] +
                        " | WASD: Move | Q/E: Zoom | Tab: Character | F1/F2: Gallery | Home: Landscape | F3: World";
                    glfwSetWindowTitle(window, title.c_str());
                }
            }
        m_Camera.Follow(m_Player, deltaTime);
    }
    void DemoScene2D::DrawActor(MyEngine::Canvas2D& canvas, int character, glm::vec2 feet, float bob)
    {
        // Ground marker and a single-pose cutout; bobbing is procedural, not a walk-cycle sheet.
        canvas.Rectangle(feet.x-22, feet.y-4, 44, 9, {0.08f,0.16f,0.09f,0.19f});
        canvas.Sprite(m_Characters[static_cast<std::size_t>(character)], feet.x-78, feet.y-150-bob, 156,156);
    }
    void DemoScene2D::Render(MyEngine::Canvas2D& canvas, int width, int height)
    {
        if (!m_Ready) return;
        m_Camera.Resize(width,height);
        canvas.BeginWorld(width,height,m_Camera);
        // Neutral surround when an unusually wide view extends beyond the painted map.
        canvas.Rectangle(-4000,-4000,8000,8000,{0.43f,0.57f,0.28f,1});
        canvas.Image(m_Map,0,0,WorldSize.x,WorldSize.y);
        constexpr std::array<glm::vec2,4> objects = {{{650,700},{1010,700},{1090,770},{970,850}}};
        for (std::size_t i=0; i<objects.size(); ++i)
            canvas.Sprite(m_Items,objects[i].x-32,objects[i].y-52,64,64,ItemRegions[i]);
        const glm::vec2 npc{990,600};
        const float bob = m_Moving ? std::sin(m_WalkTime*12.0f)*2.2f : 0;
        // Painter's ordering gives sprites a consistent overlap by their foot Y.
        if (m_Player.y < npc.y) DrawActor(canvas,m_Character,m_Player,bob);
        DrawActor(canvas,1-m_Character,npc,0);
        if (m_Player.y >= npc.y) DrawActor(canvas,m_Character,m_Player,bob);
        canvas.End();

        // Screen UI uses a different projection and therefore does not zoom with the world.
        canvas.Begin(width,height);
        const float size = std::min(78.0f, static_cast<float>(width)/5.5f);
        const float gap = size*0.12f;
        const float start = (static_cast<float>(width) - (size*4+gap*3))*0.5f;
        const float y = std::max(0.0f, static_cast<float>(height)-size-20);
        for (int i=0; i<4; ++i)
        {
            const float x = start+static_cast<float>(i)*(size+gap);
            canvas.Rectangle(x-3,y-3,size+6,size+6,
                             i==m_SelectedItem ? std::array<float,4>{0.95f,0.74f,0.32f,1} : std::array<float,4>{0.18f,0.28f,0.18f,0.7f});
            canvas.Rectangle(x,y,size,size,{0.99f,0.97f,0.88f,0.94f});
            canvas.Sprite(m_Items,x+5,y+5,size-10,size-10,ItemRegions[static_cast<std::size_t>(i)]);
        }
        canvas.Rectangle(16,16,132,76,{0.99f,0.97f,0.88f,0.92f});
        for (int i=0; i<2; ++i)
        {
            const float x=20+static_cast<float>(i)*62;
            canvas.Image(m_Characters[static_cast<std::size_t>(i)],x,20,58,58);
            if (i==m_Character) canvas.Rectangle(x,82,58,3,{0.71f,0.46f,0.14f,1});
        }
        canvas.End();
    }
    void DemoScene2D::Shutdown()
    {
        m_Ready = false;
        m_Map.Reset();
        m_Items.Reset();
        for (auto& character:m_Characters) character.Reset();
    }
}

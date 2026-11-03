#include "GameplayState.h"
#include "Resources/AssetPath.h"
#include "Input/Input.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <array>
#include <string>

namespace
{
    constexpr std::array Library = {
        "Library/SNHR_v11/Characters/character_0001/sprite.png",
        "Library/SNHR_v11/Animals/animal_0013/sprite.png",
        "Library/SNHR_v8/Crops_Plants/IMG_082/plant_05_07_trim.png",
        "Library/SNHR_v8/Environment/IMG_046/nature_prop_06_01_trim.png",
        "Library/SNHR_v8/Items_Food/IMG_021/drink_item_01_01_trim.png"
    };
    constexpr std::array History = {
        "History/Render02_Preview.png", "History/Gameplay03_Preview.png",
        "History/SaveShop04_Preview.png", "History/Audit05_Preview.png",
        "History/UI06_Inventory.png", "History/UI06_Shop.png",
        "History/Sprite07_Preview.png", "History/Visual08_Preview.png"
    };
}

namespace MyGame
{
    void GameplayState::Enter()
    {
        m_Mode = 0;
        m_Selected = 0;
        m_Ready = m_Canvas.Initialize() &&
            m_Background.Load(MyEngine::FindAsset("Backgrounds/cozy_farm_morning.png"));
        UpdateTitle();
    }

    void GameplayState::Update(float /*deltaTime*/)
    {
        constexpr std::array keys = {GLFW_KEY_F1, GLFW_KEY_F2, GLFW_KEY_HOME,
                                     GLFW_KEY_LEFT, GLFW_KEY_RIGHT};
        for (std::size_t i = 0; i < keys.size(); ++i)
        {
            if (MyEngine::Input::ConsumeKeyPress(keys[i]))
            {
                if (i == 0) SetMode(m_Mode == 1 ? 0 : 1);
                else if (i == 1) SetMode(m_Mode == 2 ? 0 : 2);
                else if (i == 2) SetMode(0);
                else if (m_Mode != 0)
                {
                    const auto count = m_Mode == 1 ? Library.size() : History.size();
                    m_Selected = (m_Selected + (i == 3 ? count - 1 : 1)) % count;
                    LoadPreview();
                }
            }
        }
    }

    void GameplayState::Render()
    {
        if (!m_Ready) return;
        int width = 0, height = 0;
        glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
        if (width <= 0 || height <= 0) return;
        m_Canvas.Begin(width, height);
        m_Canvas.Cover(m_Background);
        if (m_Mode != 0)
        {
            const auto w = static_cast<float>(width), h = static_cast<float>(height);
            m_Canvas.Rectangle(0, 0, w, h, {0.04f,0.12f,0.09f,0.64f});
            const float cardW = w * 0.82f, cardH = h * 0.82f;
            const float x = (w-cardW)*0.5f, y = (h-cardH)*0.5f;
            m_Canvas.Rectangle(x+6, y+8, cardW, cardH, {0,0,0,0.2f});
            m_Canvas.Rectangle(x, y, cardW, cardH, {0.98f,0.96f,0.9f,1});
            if (m_Preview.Id())
            {
                const float padding = std::min(28.0f, std::min(cardW, cardH)*0.08f);
                const float scale = std::min((cardW-2*padding)/static_cast<float>(m_Preview.Width()),
                                             (cardH-2*padding)/static_cast<float>(m_Preview.Height()));
                const float imageW = static_cast<float>(m_Preview.Width())*scale;
                const float imageH = static_cast<float>(m_Preview.Height())*scale;
                m_Canvas.Image(m_Preview, (w-imageW)*0.5f, (h-imageH)*0.5f, imageW, imageH);
            }
        }
        m_Canvas.End();
    }

    void GameplayState::Exit()
    {
        m_Ready = false;
        m_Preview.Reset();
        m_Background.Reset();
        m_Canvas.Shutdown();
    }

    void GameplayState::SetMode(int mode)
    {
        m_Mode = mode;
        m_Selected = 0;
        LoadPreview();
    }

    void GameplayState::LoadPreview()
    {
        m_Preview.Reset();
        if (m_Mode != 0)
        {
            const auto* path = m_Mode == 1 ? Library[m_Selected] : History[m_Selected];
            m_Preview.Load(MyEngine::FindAsset(path));
        }
        UpdateTitle();
    }

    void GameplayState::UpdateTitle()
    {
        std::string title = "My Farm | Cozy Morning | F1: Artwork | F2: History | Esc: Exit";
        if (m_Mode != 0)
        {
            const auto* path = m_Mode == 1 ? Library[m_Selected] : History[m_Selected];
            title = "My Farm | " + std::string(m_Mode == 1 ? "Asset review: " : "PAST PREVIEW (reference only): ")
                + path + (m_Preview.Id() ? "" : " [LOAD FAILED]")
                + " | Left/Right: Browse | Home: Landscape | Esc: Exit";
        }
        if (auto* window = glfwGetCurrentContext())
            glfwSetWindowTitle(window, title.c_str());
    }
}

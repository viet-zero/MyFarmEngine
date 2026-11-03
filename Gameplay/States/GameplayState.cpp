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
        "library_snhr_v11_characters_character_0001_sprite",
        "library_snhr_v11_animals_animal_0013_sprite",
        "library_snhr_v8_crops_plants_img_082_plant_05_07_trim",
        "library_snhr_v8_environment_img_046_nature_prop_06_01_trim",
        "library_snhr_v8_items_food_img_021_drink_item_01_01_trim"
    };
    constexpr std::array History = {
        "history_render02_preview", "history_gameplay03_preview",
        "history_saveshop04_preview", "history_audit05_preview",
        "history_ui06_inventory", "history_ui06_shop",
        "history_sprite07_preview", "history_visual08_preview"
    };
}

namespace MyGame
{
    void GameplayState::Enter()
    {
        m_Mode = 3;
        m_Selected = 0;
        m_Assets.Initialize(MyEngine::AssetRoot());
        m_Background=m_Assets.Load("background_morning");
        const auto root=MyEngine::AssetRoot();
        m_Ready = m_Canvas.Initialize() && m_Demo.Initialize(m_Assets,root/"Data");
        UpdateTitle();
    }

    void GameplayState::Update(float deltaTime)
    {
        constexpr std::array keys = {GLFW_KEY_F1, GLFW_KEY_F2, GLFW_KEY_HOME,
                                     GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_F3};
        for (std::size_t i = 0; i < keys.size(); ++i)
        {
            if (MyEngine::Input::ConsumeKeyPress(keys[i]))
            {
                if (i == 0) SetMode(m_Mode == 1 ? 0 : 1);
                else if (i == 1) SetMode(m_Mode == 2 ? 0 : 2);
                else if (i == 2) SetMode(0);
                else if (i == 5) SetMode(3);
                else if (m_Mode == 1 || m_Mode == 2)
                {
                    const auto count = m_Mode == 1 ? Library.size() : History.size();
                    m_Selected = (m_Selected + (i == 3 ? count - 1 : 1)) % count;
                    LoadPreview();
                }
            }
        }
        if (m_Mode == 3) m_Demo.Update(deltaTime);
    }

    void GameplayState::Render()
    {
        if (!m_Ready) return;
        int width = 0, height = 0;
        glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
        if (width <= 0 || height <= 0) return;
        if (m_Mode == 3)
        {
            m_Demo.Render(m_Canvas,width,height);
            return;
        }
        m_Canvas.Begin(width, height);
        if(m_Background) m_Canvas.Cover(*m_Background);
        else m_Canvas.Rectangle(0,0,static_cast<float>(width),static_cast<float>(height),{0.32f,0.48f,0.28f,1});
        if (m_Mode != 0)
        {
            const auto w = static_cast<float>(width), h = static_cast<float>(height);
            m_Canvas.Rectangle(0, 0, w, h, {0.04f,0.12f,0.09f,0.64f});
            const float cardW = w * 0.82f, cardH = h * 0.82f;
            const float x = (w-cardW)*0.5f, y = (h-cardH)*0.5f;
            m_Canvas.Rectangle(x+6, y+8, cardW, cardH, {0,0,0,0.2f});
            m_Canvas.Rectangle(x, y, cardW, cardH, {0.98f,0.96f,0.9f,1});
            if (m_Preview)
            {
                const float padding = std::min(28.0f, std::min(cardW, cardH)*0.08f);
                const float scale = std::min((cardW-2*padding)/static_cast<float>(m_Preview->Width()),
                                             (cardH-2*padding)/static_cast<float>(m_Preview->Height()));
                const float imageW = static_cast<float>(m_Preview->Width())*scale;
                const float imageH = static_cast<float>(m_Preview->Height())*scale;
                m_Canvas.Image(*m_Preview, (w-imageW)*0.5f, (h-imageH)*0.5f, imageW, imageH);
            }
            else m_Canvas.Rectangle(x+cardW*0.3f,y+cardH*0.3f,cardW*0.4f,cardH*0.4f,{0.6f,0.4f,0.25f,1});
        }
        m_Canvas.End();
    }

    void GameplayState::Exit()
    {
        m_Ready = false;
        m_Demo.Shutdown();
        m_Preview=m_Background=nullptr;
        m_Assets.Shutdown();
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
        m_Preview=nullptr;
        if (m_Mode == 1 || m_Mode == 2)
        {
            const auto* path = m_Mode == 1 ? Library[m_Selected] : History[m_Selected];
            m_Preview=m_Assets.Load(path);
        }
        UpdateTitle();
    }

    void GameplayState::UpdateTitle()
    {
        std::string title = "My Farm | Cozy Morning | F1: Artwork | F2: History | F3: World | Esc: Exit";
        if (m_Mode == 3)
            title = "My Farm - Phase B | WASD: Move | Shift: Run | Space: Action | H: Hurt | Tab: Character | R: Reset";
        if (m_Mode == 1 || m_Mode == 2)
        {
            const auto* path = m_Mode == 1 ? Library[m_Selected] : History[m_Selected];
            title = "My Farm | " + std::string(m_Mode == 1 ? "Asset review: " : "PAST PREVIEW (reference only): ")
                + path + (m_Preview ? "" : " [FALLBACK / NOT APPROVED]")
                + " | Left/Right: Browse | Home: Landscape | Esc: Exit";
        }
        if (auto* window = glfwGetCurrentContext())
            glfwSetWindowTitle(window, title.c_str());
    }
}

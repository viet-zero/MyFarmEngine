#include "Core/Application.h"
#include "Core/Time.h"
#include "Input/Input.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace
{
    int failures = 0;
    void Check(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << message << '\n';
            ++failures;
        }
    }
    class TestGame : public MyEngine::IGame
    {
    public:
        bool failInitialize = false;
        int starts = 0, updates = 0, renders = 0, stops = 0;
        bool Initialize() override
        {
            ++starts;
            Check(glfwGetCurrentContext() != nullptr, "game initialized without GL context");
            return !failInitialize;
        }
        void Update(float deltaTime) override
        {
            ++updates;
            Check(deltaTime >= 0.0f && deltaTime <= MyEngine::Time::MaxDeltaTime,
                  "game received invalid delta time");
            Check(updates == renders + 1, "update/render order broken");
            if (updates == 1)
                glfwSetWindowSize(glfwGetCurrentContext(), 480, 320);
        }
        void Render() override
        {
            ++renders;
            int width = 0, height = 0;
            glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
            // Resize requests can arrive during Update. The viewport must match
            // the framebuffer captured by Application before that Update.
            int viewport[4] = {};
            glGetIntegerv(GL_VIEWPORT, viewport);
            Check(viewport[2] > 0 && viewport[3] > 0, "empty viewport");
            if (renders == 3)
                Check(viewport[2] == width && viewport[3] == height, "viewport did not follow resize");
            Check(glGetError() == GL_NO_ERROR, "OpenGL error during frame");
        }
        void Shutdown() override
        {
            ++stops;
            Check(glfwGetCurrentContext() != nullptr, "GL context destroyed before game cleanup");
        }
    };
}

int main()
{
    using MyEngine::Application;
    Check(!MyEngine::Input::IsKeyPressed(GLFW_KEY_ESCAPE), "input unsafe before initialization");
    TestGame game;
    {
        Application app(game);
        app.Run(1); // Safe before initialization.
        Check(!app.Initialize(0, 100, "invalid"), "zero width accepted");
        Check(game.starts == 0, "game started after invalid window request");
        if (!app.Initialize(640, 360, "Engine lifecycle test", false))
        {
            std::cerr << "A desktop with OpenGL 3.3 is required for this test.\n";
            return 1;
        }
        Check(app.Initialize(), "repeated Initialize failed");
        Check(game.starts == 1, "repeated Initialize started game twice");
        app.Run(3);
        Check(game.updates == 3 && game.renders == 3, "frame budget was not respected");
        app.Shutdown();
        app.Shutdown();
        Check(game.stops == 1, "Shutdown called game more than once");
        Check(!MyEngine::Input::IsKeyPressed(GLFW_KEY_ESCAPE), "input retained destroyed window");
        Check(app.Initialize(320, 240, "Restart test", false), "restart failed");
        app.Run(1);
    }
    Check(game.starts == 2 && game.stops == 2, "destructor or restart lifecycle broken");
    TestGame failingGame;
    failingGame.failInitialize = true;
    {
        Application app(failingGame);
        Check(!app.Initialize(320, 240, "Initialization failure test", false),
              "game initialization failure ignored");
        app.Run(1);
        Check(failingGame.stops == 1, "partial game initialization not cleaned up");
    }
    Check(failingGame.updates == 0 && failingGame.stops == 1,
          "failed game ran or was cleaned up twice");
    return failures == 0 ? 0 : 1;
}

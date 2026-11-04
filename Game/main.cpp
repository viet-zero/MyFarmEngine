#include "Core/Application.h"
#include "Game/GameApp/Game.h"
#include "Game/GameApp/GameConfig.h"
#include "Game/RuntimeLog.h"
#include <exception>
#include <iostream>
#include <string_view>
#ifdef MYFARM_WINDOWED_APP
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstdlib>
#endif

namespace
{
    void ShowFailure(const char* message)
    {
        std::cerr<<message<<'\n';
#ifdef MYFARM_WINDOWED_APP
        MessageBoxA(nullptr,message,"MyFarmEngine",MB_OK|MB_ICONERROR);
#endif
    }
    int Run(int argc,char** argv)
    {
        MyGame::RuntimeLog log;
        if(!log.Ready())
        {
            ShowFailure("Cannot open runtime log. Check MYFARM_LOG_DIR or LocalAppData permissions.");
            return 1;
        }
        try
        {
            unsigned int maxFrames=0;
            MyGame::GameConfig config;
            for(int i=1;i<argc;++i)
            {
                const std::string_view argument(argv[i]);
                if(argument=="--smoke-test") maxFrames=5;
                else if(argument=="--no-vsync") config.vsync=false;
                else if(argument=="--demo") config.scene.demoMode=true;
                else if(argument=="--npc-ai") config.scene.npcAI=true;
                else if(argument=="--pet-follow") config.scene.petFollow=true;
                else if(argument=="--debug-overlay") config.scene.debugOverlay=true;
                else
                {
                    std::cerr<<"Usage: MyFarmEngine [--smoke-test] [--no-vsync] [--demo] [--npc-ai] [--pet-follow] [--debug-overlay]\n";
                    return 2;
                }
            }
            MyGame::Game game(config.scene);
            MyEngine::Application app(game);
            if(!app.Initialize(config.windowWidth,config.windowHeight,config.title,config.vsync))
            {
                ShowFailure("Application initialization failed. See MyFarmEngine logs in LocalAppData (or MYFARM_LOG_DIR).");
                return 1;
            }
            app.Run(maxFrames);
            app.Shutdown();
            std::cout<<"Normal shutdown\n";
            return 0;
        }
        catch(const std::exception& error) { ShowFailure(error.what());return 1; }
    }
}
#ifdef MYFARM_WINDOWED_APP
// The Windows CRT still parses the command line. No custom /ENTRY or CRT bypass.
int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int) { return Run(__argc,__argv); }
#else
int main(int argc,char** argv) { return Run(argc,argv); }
#endif

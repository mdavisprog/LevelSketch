#include <cstdio>
#include "OctaneGUI/OctaneGUI.h"
#include "Core/Defines.hpp"
#include "Platform/Window.hpp"
#include "Render/Renderer.hpp"

#ifdef WINDOWS
    #include "Platform/Windows/Platform.hpp"
    #include "Render/DirectX/Renderer.hpp"
#endif

#ifdef WITH_TESTS
    #include "Tests/System.hpp"
#endif

static LevelSketch::Platform::Platform* g_Platform { nullptr };
static LevelSketch::Render::Renderer* g_Renderer { nullptr };
static std::unordered_map<OctaneGUI::Window*, LevelSketch::Platform::Window*> g_Windows {};

void OnWindowAction(OctaneGUI::Window* Window, OctaneGUI::WindowAction Action)
{
    switch (Action)
    {
    case OctaneGUI::WindowAction::Create:
    {
        if (g_Windows.find(Window) == g_Windows.end())
        {
            LevelSketch::Platform::Window* Win = g_Platform->NewWindow(
                OctaneGUI::String::ToMultiByte(Window->GetTitle()).c_str(),
                (int)Window->GetPosition().X,
                (int)Window->GetPosition().Y,
                (int)Window->GetSize().X,
                (int)Window->GetSize().Y
            );

            if (Win != nullptr)
            {
                g_Windows[Window] = Win;
                Win->Show();
                g_Renderer->SetWindow(Win);
                g_Renderer->Initialize();
            }
            else
            {
                printf("Failed to create window!\n");
            }
        }
    } break;

    case OctaneGUI::WindowAction::Destroy:
    {
        if (g_Windows.find(Window) != g_Windows.end())
        {
            g_Platform->CloseWindow(g_Windows[Window]);
            g_Windows.erase(Window);
        }
    } break;

    case OctaneGUI::WindowAction::Raise:
    {
        if (g_Windows.find(Window) != g_Windows.end())
        {
            g_Windows[Window]->Focus();
        }
    } break;

    case OctaneGUI::WindowAction::Position:
    {
        if (g_Windows.find(Window) != g_Windows.end())
        {
            g_Windows[Window]->SetPosition((int)Window->GetPosition().X, (int)Window->GetPosition().Y);
        }
    } break;

    case OctaneGUI::WindowAction::Enable:
    case OctaneGUI::WindowAction::Disable:
    case OctaneGUI::WindowAction::Size:
    case OctaneGUI::WindowAction::Minimize:
    case OctaneGUI::WindowAction::Maximize:
    default: break;
    }
}

OctaneGUI::Event OnEvent(OctaneGUI::Window* Window)
{
    if (g_Windows.find(Window) == g_Windows.end())
    {
        return { OctaneGUI::Event::Type::WindowClosed };
    }

    LevelSketch::Platform::Window* Win { g_Windows[Window] };
    if (!Win->IsOpen())
    {
        g_Windows.erase(Window);
        return { OctaneGUI::Event::Type::WindowClosed };
    }

    Win->ProcessEvents();
    g_Renderer->Render();

    return { OctaneGUI::Event::Type::None };
}

int main(int argc, char** argv)
{
#ifdef WITH_TESTS
    for (int I = 0; I < argc; I++)
    {
        if (std::string{argv[I]} == "--tests")
        {
            return LevelSketch::Tests::System::Instance().Run(argc, argv);
        }
    }
#endif

    const char* Stream = R"({
        "Windows": {
            "Main": {"Title": "Level Sketch", "Width": 1280, "Height": 720,
                "MenuBar": {},
                "Body": {"Controls": []}
            }
        }
    })";

#ifdef WINDOWS
    g_Platform = new LevelSketch::Platform::Windows::Platform();
    
    if (!g_Platform->Initialize())
    {
        delete g_Platform;
        printf("Failed to initialize platform!\n");
        return -1;
    }

    g_Renderer = new LevelSketch::Render::DirectX::Renderer();
#endif

    OctaneGUI::Application Application;
    Application
        .SetOnWindowAction(OnWindowAction)
        .SetOnEvent(OnEvent);

    std::unordered_map<std::string, OctaneGUI::ControlList> List;
    bool Success = Application
        .SetCommandLine(argc, argv)
        .Initialize(Stream, List);
    
    if (!Success)
    {
        printf("Failed to initialize application!\n");
        return 0;
    }

    int Return { Application.Run() };

    for (const std::pair<OctaneGUI::Window*, LevelSketch::Platform::Window*> Item : g_Windows)
    {
        g_Platform->CloseWindow(Item.second);
    }
    g_Windows.clear();

    g_Renderer->Shutdown();
    delete g_Renderer;

    g_Platform->Shutdown();
    delete g_Platform;

    return Return;
}

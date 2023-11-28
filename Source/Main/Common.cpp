/**

MIT License

Copyright (c) 2023 Mitchell Davis <mdavisprog@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "Common.hpp"
#include <cstdio>
#include "../External/OctaneGUI/OctaneGUI.h"
#include "../Core/Defines.hpp"
#include "../Platform/Platform.hpp"
#include "../Platform/Window.hpp"
#include "../Render/Renderer.hpp"

#ifdef WITH_TESTS
    #include "../Tests/System.hpp"
#endif

namespace LevelSketch
{
namespace Main
{

static OctaneGUI::Application* g_Application { nullptr };
static std::unordered_map<OctaneGUI::Window*, LevelSketch::Platform::Window*> g_Windows {};

void OnWindowAction(OctaneGUI::Window* Window, OctaneGUI::WindowAction Action)
{
    switch (Action)
    {
    case OctaneGUI::WindowAction::Create:
    {
        if (g_Windows.find(Window) == g_Windows.end())
        {
            LevelSketch::Platform::Window* Win = Platform::Platform::Instance()->NewWindow(
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
                Render::Renderer::Instance()->Initialize(Win);
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
            Platform::Platform::Instance()->CloseWindow(g_Windows[Window]);
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
    Render::Renderer::Instance()->Render(Win);

    return { OctaneGUI::Event::Type::None };
}

void OnPlatformFrame()
{
}

i32 Main(i32 Argc, char** Argv)
{
#if defined(WITH_TESTS)
    for (int I = 0; I < Argc; I++)
    {
        if (std::string{Argv[I]} == "--tests")
        {
            return LevelSketch::Tests::System::Instance().Run(Argc, Argv);
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

    const Core::Memory::UniquePtr<Platform::Platform>& Platform { Platform::Platform::Instance() };
    if (!Platform->Initialize())
    {
        printf("Failed to initialize platform!\n");
        return -1;
    }

    if (!Render::Renderer::Instance()->Initialize())
    {
        printf("Failed to initialize renderer!\n");
        return -1;
    }

    g_Application = new OctaneGUI::Application();
    g_Application
        ->SetOnWindowAction(OnWindowAction)
        .SetOnEvent(OnEvent);

    std::unordered_map<std::string, OctaneGUI::ControlList> List;
    bool Success = g_Application
        ->SetCommandLine(Argc, Argv)
        .Initialize(Stream, List);
    
    if (!Success)
    {
        printf("Failed to initialize application!\n");
        return 0;
    }

    int Return { 0 };
    if (Platform->UseCustomLoop())
    {
        Platform->SetOnFrame(OnPlatformFrame);
        Return = Platform->Run();
    }
    else
    {
        Return = g_Application->Run();
    }

    for (const std::pair<OctaneGUI::Window*, LevelSketch::Platform::Window*> Item : g_Windows)
    {
        Platform->CloseWindow(Item.second);
    }
    g_Windows.clear();

    g_Application->Shutdown();
    delete g_Application;

    return Return;
}

}
}

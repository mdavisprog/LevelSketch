/**

MIT License

Copyright (c) 2024 Mitchell Davis <mdavisprog@gmail.com>

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
#include "../Core/CommandLine.hpp"
#include "../Core/Console.hpp"
#include "../Core/Defines.hpp"
#include "../Engine/Engine.hpp"
#include "../External/OctaneGUI/OctaneGUI.h"
#include "../Platform/Debugger.hpp"
#include "../Platform/EventQueue.hpp"
#include "../Platform/FileSystem.hpp"
#include "../Platform/Platform.hpp"
#include "../Platform/Window.hpp"
#include "../Render/Renderer.hpp"
#include <cstdio>

#ifdef WITH_TESTS
#include "../Tests/System.hpp"
#endif

namespace LevelSketch
{
namespace Main
{

struct UIEvent
{
public:
    OctaneGUI::Event Event { OctaneGUI::Event::Type::None };
    Platform::Window* Window { nullptr };
};

static OctaneGUI::Application* g_Application { nullptr };
static std::unordered_map<OctaneGUI::Window*, LevelSketch::Platform::Window*> g_Windows {};
static Array<UIEvent> g_UIEvents {};

static OctaneGUI::Mouse::Button Transform(const Platform::Mouse::Button::Type Button)
{
    switch (Button)
    {
    case Platform::Mouse::Button::Middle: return OctaneGUI::Mouse::Button::Middle;
    case Platform::Mouse::Button::Right: return OctaneGUI::Mouse::Button::Right;
    case Platform::Mouse::Button::Left:
    case Platform::Mouse::Button::None:
    default: break;
    }

    return OctaneGUI::Mouse::Button::Left;
}

static OctaneGUI::Event Transform(const Platform::Event& Event)
{
    switch (Event.GetType())
    {
    case Platform::Event::Type::MouseMove:
    {
        const Platform::Event::OnMouseMove& Data { Event.GetData().MouseMove };
        return OctaneGUI::Event(
            OctaneGUI::Event::MouseMove(static_cast<f32>(Data.Position.X), static_cast<f32>(Data.Position.Y)));
    }
    break;

    case Platform::Event::Type::MouseButton:
    {
        const Platform::Event::OnMouseButton& Data { Event.GetData().MouseButton };
        const OctaneGUI::Event::Type Type { Data.Pressed ? OctaneGUI::Event::Type::MousePressed
                                                         : OctaneGUI::Event::Type::MouseReleased };
        return OctaneGUI::Event(Type,
            OctaneGUI::Event::MouseButton(Transform(Data.Button),
                static_cast<f32>(Data.Position.X),
                static_cast<f32>(Data.Position.Y),
                OctaneGUI::Mouse::Count::Single));
    }
    break;

    case Platform::Event::Type::None:
    default: break;
    }

    return OctaneGUI::Event(OctaneGUI::Event::Type::None);
}

static void OnWindowAction(OctaneGUI::Window* Window, OctaneGUI::WindowAction Action)
{
    switch (Action)
    {
    case OctaneGUI::WindowAction::Create:
    {
        if (g_Windows.find(Window) == g_Windows.end())
        {
            Platform::Window* Win =
                Platform::Platform::Instance()->NewWindow(OctaneGUI::String::ToMultiByte(Window->GetTitle()).c_str(),
                    (int)Window->GetPosition().X,
                    (int)Window->GetPosition().Y,
                    (int)Window->GetSize().X,
                    (int)Window->GetSize().Y);

            if (Win != nullptr)
            {
                g_Windows[Window] = Win;
                Win->Show();

                const bool WasInitialized { Render::Renderer::Instance()->Initialized() };
                if (Render::Renderer::Instance()->Initialize(Win))
                {
                    if (WasInitialized != Render::Renderer::Instance()->Initialized())
                    {
                        printf("Rendering Driver Summary\n");
                        printf("Vendor: %s\n", Render::Renderer::Instance()->Summary().Vendor.Data());
                        printf("Renderer: %s\n", Render::Renderer::Instance()->Summary().Renderer.Data());
                        printf("Version: %s\n", Render::Renderer::Instance()->Summary().Version.Data());
                        printf("Shading Language Version: %s\n",
                            Render::Renderer::Instance()->Summary().ShadingLanguageVersion.Data());
                    }

                    const Vector2 Scale { Win->ContentScale() };
                    Window->SetRenderScale({ Scale.X, Scale.Y });
                }
                else
                {
                    Win->Close();
                    Window->Close();
                }
            }
            else
            {
                printf("Failed to create window!\n");
            }
        }
    }
    break;

    case OctaneGUI::WindowAction::Destroy:
    {
        if (g_Windows.find(Window) != g_Windows.end())
        {
            Platform::Platform::Instance()->CloseWindow(g_Windows[Window]);
            g_Windows.erase(Window);
        }
    }
    break;

    case OctaneGUI::WindowAction::Raise:
    {
        if (g_Windows.find(Window) != g_Windows.end())
        {
            g_Windows[Window]->Focus();
        }
    }
    break;

    case OctaneGUI::WindowAction::Position:
    {
        if (g_Windows.find(Window) != g_Windows.end())
        {
            g_Windows[Window]->SetPosition((int)Window->GetPosition().X, (int)Window->GetPosition().Y);
        }
    }
    break;

    case OctaneGUI::WindowAction::Enable:
    case OctaneGUI::WindowAction::Disable:
    case OctaneGUI::WindowAction::Size:
    case OctaneGUI::WindowAction::Minimize:
    case OctaneGUI::WindowAction::Maximize:
    default: break;
    }
}

static OctaneGUI::Event OnEvent(OctaneGUI::Window* Window)
{
    if (g_Windows.find(Window) == g_Windows.end())
    {
        return { OctaneGUI::Event::Type::WindowClosed };
    }

    // FIXME: Find a way better way to map a platform window with an OctaneGUI window.
    LevelSketch::Platform::Window* Win { g_Windows[Window] };
    if (!LevelSketch::Platform::Platform::Instance()->HasWindow(Win) || !Win->IsOpen())
    {
        g_Windows.erase(Window);
        return { OctaneGUI::Event::Type::WindowClosed };
    }

    for (u64 I = 0; I < g_UIEvents.Size(); I++)
    {
        const UIEvent Event { g_UIEvents[I] };
        if (Event.Window == Win)
        {
            g_UIEvents.Remove(I);
            return Event.Event;
        }
    }

    Win->ProcessEvents();

    return { OctaneGUI::Event::Type::None };
}

static u32 OnLoadTexture(const std::vector<u8>& Data, u32 Width, u32 Height)
{
    return Render::Renderer::Instance()->LoadTexture(Data.data(), Width, Height);
}

static void OnPaint(OctaneGUI::Window* Window, const OctaneGUI::VertexBuffer& Buffer)
{
    Render::Renderer::Instance()->UploadGUIData(Window, Buffer);
}

static bool OnPlatformFrame(const Platform::TimingData&)
{
    if (!g_Application->IsRunning())
    {
        return false;
    }

    Array<Platform::Event> Events { Platform::EventQueue::Instance().Consume() };
    for (const Platform::Event& Event : Events)
    {
        g_UIEvents.Push({ Transform(Event), Event.GetWindow() });
    }

    g_Application->RunFrame();

    for (const UniquePtr<Platform::Window>& Window : Platform::Platform::Instance()->Windows())
    {
        Render::Renderer::Instance()->Render(Window.Get());
    }

    return true;
}

i32 Main(i32 Argc, const char** Argv)
{
    Core::CommandLine::Instance().Set(Argc, Argv);
    Platform::Debugger::Instance()->Initialize();

#if defined(WITH_TESTS)
    if (Core::CommandLine::Instance().Has("--tests"))
    {
        return LevelSketch::Tests::System::Instance().Run();
    }
#endif

#if defined(DEBUG)
    if (Core::CommandLine::Instance().Has("--appcwd"))
    {
        Platform::FileSystem::SetWorkingDirectory(Platform::FileSystem::ApplicationDirectory());
    }
#endif

    const char* Stream = R"({
        "Theme": {
            "FontPath": "Content/Fonts/Roboto-Regular.ttf",
            "FontSize": 16
        },
        "Windows": {
            "Main": {"Title": "Level Sketch", "Width": 1280, "Height": 720,
                "MenuBar": {"Items": [
                    {"Text": "File"}
                ]},
                "Body": {"Controls": []}
            }
        }
    })";

    const UniquePtr<Platform::Platform>& Platform { Platform::Platform::Instance() };
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
    g_Application->SetOnWindowAction(OnWindowAction)
        .SetOnEvent(OnEvent)
        .SetOnLoadTexture(OnLoadTexture)
        .SetOnPaint(OnPaint);

    std::unordered_map<std::string, OctaneGUI::ControlList> List;
    bool Success = g_Application->SetCommandLine(Argc, const_cast<char**>(Argv)).Initialize(Stream, List);

    if (!Success)
    {
        printf("Failed to initialize application!\n");
        return 0;
    }

    if (!Engine::Engine::Instance().Initialize())
    {
        Core::Console::Error("Failed to initialize engine!");
        return 0;
    }

    int Return = Platform->SetOnFrame(OnPlatformFrame).Run();

    Engine::Engine::Instance().Shutdown();

    for (const std::pair<OctaneGUI::Window*, LevelSketch::Platform::Window*> Item : g_Windows)
    {
        if (Platform->HasWindow(Item.second))
        {
            Platform->CloseWindow(Item.second);
        }
    }
    g_Windows.clear();

    g_Application->Shutdown();
    delete g_Application;

    Platform->Shutdown();
    Platform::Debugger::Instance()->Shutdown();

    return Return;
}

}
}

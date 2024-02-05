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

#include "GUI.hpp"
#include "../Core/Console.hpp"
#include "../Core/Math/Vector2.hpp"
#include "../External/OctaneGUI/OctaneGUI.h"
#include "../Platform/Platform.hpp"
#include "../Platform/Window.hpp"
#include "../Render/Renderer.hpp"

#include <unordered_map>
#include <vector>

namespace LevelSketch
{
namespace GUI
{

static OctaneGUI::Mouse::Button Transform(const Platform::Mouse::Button::Type Button)
{
    switch (Button)
    {
    case Platform::Mouse::Button::Middle: return OctaneGUI::Mouse::Button::Middle;
    case Platform::Mouse::Button::Right: return OctaneGUI::Mouse::Button::Right;
    case Platform::Mouse::Button::Left: return OctaneGUI::Mouse::Button::Left;
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

GUI& GUI::Instance()
{
    static GUI Instance;
    return Instance;
}

bool GUI::Initialize(i32 Argc, const char** Argv)
{
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

    m_Application = UniquePtr<OctaneGUI::Application>::New();
    m_Application
        ->SetOnWindowAction(
            [this](OctaneGUI::Window* Window, OctaneGUI::WindowAction Action) -> void
            {
                OnWindowAction(Window, Action);
            })
        .SetOnEvent(
            [this](OctaneGUI::Window* Window) -> OctaneGUI::Event
            {
                return OnEvent(Window);
            })
        .SetOnLoadTexture(
            [this](const std::vector<u8>& Data, u32 Width, u32 Height) -> u32
            {
                return Render::Renderer::Instance()->LoadTexture(Data.data(), Width, Height);
            })
        .SetOnPaint(
            [this](OctaneGUI::Window* Window, const OctaneGUI::VertexBuffer& Buffer) -> void
            {
                Render::Renderer::Instance()->UploadGUIData(Window, Buffer);
            })
        .SetCommandLine(Argc, const_cast<char**>(Argv));

    std::unordered_map<std::string, OctaneGUI::ControlList> List;
    if (!m_Application->Initialize(Stream, List))
    {
        Core::Console::Error("Failed initialize GUI.");
        return false;
    }

    return true;
}

void GUI::Shutdown()
{
    for (OctaneGUI::Window* Key : m_Windows.Keys())
    {
        Platform::Window* Window { m_Windows[Key] };

        if (Platform::Platform::Instance()->HasWindow(Window))
        {
            Platform::Platform::Instance()->CloseWindow(Window);
        }
    }

    m_Windows.Clear();

    m_Application->Shutdown();
}

bool GUI::IsRunning() const
{
    return m_Application->IsRunning();
}

void GUI::RunFrame()
{
    m_Application->RunFrame();
}

GUI& GUI::PushEvent(const Platform::Event& Event)
{
    m_Events.Push(Event);
    return *this;
}

GUI::GUI()
{
}

void GUI::OnWindowAction(OctaneGUI::Window* Window, OctaneGUI::WindowAction Action)
{
    switch (Action)
    {
    case OctaneGUI::WindowAction::Create:
    {
        if (!m_Windows.Contains(Window))
        {
            Platform::Window* Win =
                Platform::Platform::Instance()->NewWindow(OctaneGUI::String::ToMultiByte(Window->GetTitle()).c_str(),
                    (int)Window->GetPosition().X,
                    (int)Window->GetPosition().Y,
                    (int)Window->GetSize().X,
                    (int)Window->GetSize().Y);

            if (Win != nullptr)
            {
                m_Windows[Window] = Win;
                Win->Show();

                const bool WasInitialized { Render::Renderer::Instance()->Initialized() };
                if (Render::Renderer::Instance()->Initialize(Win))
                {
                    if (WasInitialized != Render::Renderer::Instance()->Initialized())
                    {
                        Core::Console::WriteLine("Rendering Driver Summary");
                        Core::Console::WriteLine("Vendor: %s", Render::Renderer::Instance()->Summary().Vendor.Data());
                        Core::Console::WriteLine("Renderer: %s",
                            Render::Renderer::Instance()->Summary().Renderer.Data());
                        Core::Console::WriteLine("Version: %s", Render::Renderer::Instance()->Summary().Version.Data());
                        Core::Console::WriteLine("Shading Language Version: %s",
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
                Core::Console::Error("Failed to create window!");
            }
        }
    }
    break;

    case OctaneGUI::WindowAction::Destroy:
    {
        if (m_Windows.Contains(Window))
        {
            Platform::Window* Target { m_Windows[Window] };
            Platform::Platform::Instance()->CloseWindow(Target);
            m_Windows.Remove(Window);
        }
    }
    break;

    case OctaneGUI::WindowAction::Raise:
    {
        if (m_Windows.Contains(Window))
        {
            m_Windows[Window]->Focus();
        }
    }
    break;

    case OctaneGUI::WindowAction::Position:
    {
        if (m_Windows.Contains(Window))
        {
            m_Windows[Window]->SetPosition(static_cast<i32>(Window->GetPosition().X),
                static_cast<i32>(Window->GetPosition().Y));
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

OctaneGUI::Event GUI::OnEvent(OctaneGUI::Window* Window)
{
    if (!m_Windows.Contains(Window))
    {
        return { OctaneGUI::Event::Type::WindowClosed };
    }

    Platform::Window* Win { m_Windows[Window] };

    // FIXME: Find a way better way to map a platform window with an OctaneGUI window.
    if (!Platform::Platform::Instance()->HasWindow(Win) || !Win->IsOpen())
    {
        m_Windows.Remove(Window);
        return { OctaneGUI::Event::Type::WindowClosed };
    }

    const Platform::Event Event { PopEvent(Win) };
    const OctaneGUI::Event Result { Transform(Event) };
    if (Result.GetType() != OctaneGUI::Event::Type::None)
    {
        return Result;
    }

    Win->ProcessEvents();

    return Result;
}

Platform::Event GUI::PopEvent(Platform::Window* Window)
{
    for (u64 Index = 0; Index < m_Events.Size(); Index++)
    {
        if (m_Events[Index].GetWindow() == Window)
        {
            Platform::Event Result { m_Events[Index] };
            m_Events.Remove(Index);
            return Result;
        }
    }

    return Platform::Event();
}

}
}

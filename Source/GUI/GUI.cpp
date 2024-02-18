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
#include "../Core/Math/Rect.hpp"
#include "../Core/Math/Vector2.hpp"
#include "../External/OctaneGUI/OctaneGUI.h"
#include "../Platform/Platform.hpp"
#include "../Platform/Window.hpp"
#include "../Render/GraphicsPipelineDescription.hpp"
#include "../Render/Renderer.hpp"
#include "../Render/TextureDescription.hpp"
#include "../Render/VertexBufferDescription.hpp"
#include "../Render/VertexDataDescription.hpp"

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
            [](const std::vector<u8>& Data, u32 Width, u32 Height) -> u32
            {
                return Render::Renderer::Instance()->CreateTexture(
                    { const_cast<u8*>(Data.data()), Width, Height, Render::TextureFormat::RGBAByte });
            })
        .SetOnPaint(
            [this](OctaneGUI::Window*, const OctaneGUI::VertexBuffer& Buffer) -> void
            {
                m_LastBuffer.Data = Buffer;
                m_LastBuffer.Uploaded = false;
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

bool GUI::InitializeResources()
{
    const UniquePtr<Render::Renderer>& Renderer { Render::Renderer::Instance() };

    Render::GraphicsPipelineDescription GUIDesc {};
    GUIDesc.Name = "GUI";
    GUIDesc.CullMode = Render::CullModeType::None;
    GUIDesc.UseDepthStencilBuffer = false;
    GUIDesc.UseAlphaBlending = true;
    GUIDesc.VertexShader.Name = "GUIVS";
    GUIDesc.VertexShader.Path = "GUIVS";
    GUIDesc.VertexShader.Function = "Main";
    GUIDesc.VertexShader.VertexDescriptions.Push({ "POSITION", Render::VertexFormat::Float2 });
    GUIDesc.VertexShader.VertexDescriptions.Push({ "TEXCOORD", Render::VertexFormat::Float2 });
    GUIDesc.VertexShader.VertexDescriptions.Push({ "COLOR", Render::VertexFormat::Byte4 });
    GUIDesc.FragmentShader.Name = "GUIFS";
    GUIDesc.FragmentShader.Path = "GUIPS";
    GUIDesc.FragmentShader.Function = "Main";
    m_GUIPipeline = Renderer->CreateGraphicsPipeline(GUIDesc);
    if (m_GUIPipeline == 0)
    {
        return false;
    }

    Render::VertexBufferDescription GUIBufferDesc {};
    GUIBufferDesc.VertexBufferSize = 100000;
    GUIBufferDesc.IndexBufferSize = 100000;
    GUIBufferDesc.Stride = sizeof(OctaneGUI::Vertex);
    GUIBufferDesc.IndexFormat = Render::IndexFormatType::U32;
    m_GUIBuffer = Renderer->CreateVertexBuffer(GUIBufferDesc);
    if (m_GUIBuffer == 0)
    {
        return false;
    }

    u8 WhiteTexture[4] { 0xFF, 0xFF, 0xFF, 0xFF };
    m_WhiteTexture = Renderer->CreateTexture({ WhiteTexture, 1, 1, Render::TextureFormat::RGBAByte });

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

    if (!m_LastBuffer.Uploaded)
    {
        Render::VertexDataDescription Data {};
        Data.VertexData = const_cast<OctaneGUI::Vertex*>(m_LastBuffer.Data.GetVertices().data());
        Data.VertexDataSize = static_cast<u64>(m_LastBuffer.Data.GetVertexCount()) * sizeof(OctaneGUI::Vertex);
        Data.IndexData = const_cast<u32*>(m_LastBuffer.Data.GetIndices().data());
        Data.IndexDataSize = static_cast<u64>(m_LastBuffer.Data.GetIndexCount()) * sizeof(u32);

        if (!Render::Renderer::Instance()->UploadVertexData(m_GUIBuffer, Data))
        {
            return;
        }

        m_LastBuffer.Uploaded = true;
    }
}

void GUI::Render(Platform::Window* Window)
{
    if (!m_LastBuffer.Uploaded)
    {
        return;
    }

    const UniquePtr<Render::Renderer>& Renderer { Render::Renderer::Instance() };

    Renderer->BindGraphicsPipeline(m_GUIPipeline);
    Renderer->BindVertexBuffer(m_GUIBuffer);

    for (const OctaneGUI::DrawCommand& Command : m_LastBuffer.Data.Commands())
    {
        u32 Texture { m_WhiteTexture };
        if (Command.TextureID() != 0)
        {
            Texture = Command.TextureID();
        }

        const Vector2 Scale { Window->ContentScale() };
        Recti Clip { 0, 0, Window->Size().X * static_cast<i32>(Scale.X), Window->Size().Y * static_cast<i32>(Scale.Y) };
        if (!Command.Clip().IsZero())
        {
            Clip.X = static_cast<i32>(Command.Clip().Min.X * Scale.X);
            Clip.Y = static_cast<i32>(Command.Clip().Min.Y * Scale.Y);
            Clip.W = static_cast<i32>(Command.Clip().Width() * Scale.X);
            Clip.H = static_cast<i32>(Command.Clip().Height() * Scale.Y);
        };

        Renderer->BindTexture(Texture);
        Renderer->SetScissor(Clip);
        Renderer->DrawIndexed(Command.IndexCount(), 1, Command.IndexOffset(), Command.VertexOffset(), 0);
    }
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

                if (Render::Renderer::Instance()->Initialize(Win))
                {
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

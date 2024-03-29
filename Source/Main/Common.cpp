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
#include "../Core/Math/Vertex.hpp"
#include "../Engine/Camera.hpp"
#include "../Engine/Components/Mesh.hpp"
#include "../Engine/Components/Transform.hpp"
#include "../Engine/ECS/World.hpp"
#include "../Engine/Engine.hpp"
#include "../GUI/GUI.hpp"
#include "../Platform/Debugger.hpp"
#include "../Platform/EventQueue.hpp"
#include "../Platform/FileSystem.hpp"
#include "../Platform/Platform.hpp"
#include "../Platform/Window.hpp"
#include "../Render/GraphicsPipelineDescription.hpp"
#include "../Render/Handle.hpp"
#include "../Render/Renderer.hpp"
#include "../Render/TextureDescription.hpp"
#include "../Render/VertexBufferDescription.hpp"
#include "../Render/VertexDataDescription.hpp"
#include <cstdio>

#ifdef WITH_TESTS
#include "../Tests/System.hpp"
#endif

namespace LevelSketch
{
namespace Main
{

static Render::GraphicsPipelineHandle g_TestPipeline {};
static Render::TextureHandle g_DefaultTexture {};

static bool g_RotateCamera { false };
static Vector2i g_LastMousePos {};
static Vector2i g_LockedMousePos {};

static Array<u8> GenerateTexture(u32 Width, u32 Height)
{
    const u32 RowPitch { Width * 4 };
    const u32 CellPitch { RowPitch >> 3 };
    const u32 CellHeight { Width >> 3 };

    Array<u8> Result;
    Result.Resize(RowPitch * Height);

    u8* Data = Result.Data();
    for (u32 I = 0; I < Result.Size(); I += 4)
    {
        const u32 X { I % RowPitch };
        const u32 Y { I / RowPitch };
        const u32 U { X / CellPitch };
        const u32 V { Y / CellHeight };

        if (U % 2 == V % 2)
        {
            Data[I] = 0x00;
            Data[I + 1] = 0x00;
            Data[I + 2] = 0x00;
            Data[I + 3] = 0xFF;
        }
        else
        {
            Data[I] = 0xFF;
            Data[I + 1] = 0xFF;
            Data[I + 2] = 0xFF;
            Data[I + 3] = 0xFF;
        }
    }

    return Result;
}

static bool IsEditorWindow(Platform::Window* Window)
{
    return Platform::Platform::Instance()->Windows()[0] == Window;
}

static void HandleKeyEvent(const Platform::Event::OnKey& OnKey)
{
    u8 Flags { Engine::Camera::Movement::None };
    switch (OnKey.Key)
    {
    case Platform::Keyboard::Key::A: Flags |= Engine::Camera::Movement::Left; break;
    case Platform::Keyboard::Key::D: Flags |= Engine::Camera::Movement::Right; break;
    case Platform::Keyboard::Key::W: Flags |= Engine::Camera::Movement::Forward; break;
    case Platform::Keyboard::Key::S: Flags |= Engine::Camera::Movement::Backward; break;
    default: break;
    }

    if (OnKey.Pressed)
    {
        Engine::Engine::Instance().GetCamera()->SetMovement(Flags);
    }
    else
    {
        Engine::Engine::Instance().GetCamera()->ClearMovement(Flags);
    }
}

static void HandleEvent(const Platform::Event& Event)
{
    switch (Event.GetType())
    {
    case Platform::Event::Type::Key:
    {
        HandleKeyEvent(Event.GetData().Key);
    }
    break;

    case Platform::Event::Type::MouseButton:
    {
        if (Event.GetData().MouseButton.Button == Platform::Mouse::Button::Left && IsEditorWindow(Event.GetWindow()))
        {
            g_RotateCamera = Event.GetData().MouseButton.Pressed;

            if (g_RotateCamera)
            {
                Platform::Mouse::Hide();
                Platform::Mouse::SetMoveMode(Platform::Mouse::MoveMode::Relative);
                g_LockedMousePos = Event.GetData().MouseButton.Position;
            }
            else
            {
                Platform::Mouse::Show();
                Platform::Mouse::SetMoveMode(Platform::Mouse::MoveMode::Absolute);
            }
        }
    }
    break;

    case Platform::Event::Type::MouseMove:
    {
        const Platform::Event::OnMouseMove& OnMouseMove { Event.GetData().MouseMove };
        Vector2i MousePos { OnMouseMove.Position };
        const Vector2i MouseDelta { MousePos - g_LastMousePos };

        if (g_RotateCamera)
        {
            Engine::Engine::Instance()
                .GetCamera()
                ->Yaw(static_cast<f32>(MouseDelta.X))
                .Pitch(static_cast<f32>(-MouseDelta.Y));
            Platform::Mouse::SetPosition(Event.GetWindow(), g_LockedMousePos);
            MousePos = g_LockedMousePos;
        }

        g_LastMousePos = MousePos;
    }
    break;

    default: break;
    }
}

static bool OnPlatformFrame(const Platform::TimingData& TimingData)
{
    GUI::GUI& GUI { GUI::GUI::Instance() };

    if (!GUI.IsRunning())
    {
        return false;
    }

    Array<Platform::Event> Events { Platform::EventQueue::Instance().Consume() };
    for (const Platform::Event& Event : Events)
    {
        GUI.PushEvent(Event);
        HandleEvent(Event);
    }

    Engine::Engine::Instance().Update(TimingData.DeltaSeconds);

    GUI.RunFrame();

    const Array<UniquePtr<Platform::Window>>& Windows { Platform::Platform::Instance()->Windows() };

    if (Windows.IsEmpty())
    {
        return true;
    }

    const UniquePtr<Render::Renderer>& Renderer { Render::Renderer::Instance() };

    // The first window is the level editor window.
    Platform::Window* Editor { Windows[0].Get() };

    if (Renderer->BeginRender(Editor, { 0.0f, 0.2f, 0.4f, 1.0f }))
    {
        Renderer->BindGraphicsPipeline(g_TestPipeline);
        Renderer->BindTexture(g_DefaultTexture);
        Engine::Engine::Instance().Render();
        Renderer->EndRender(Editor);
    }

    if (Windows.Size() > 1 && GUI::GUI::Instance().ShouldRepaint())
    {
        for (u64 I = 1; I < Windows.Size(); I++)
        {
            Platform::Window* Window { Windows[I].Get() };

            if (Renderer->BeginRender(Window, { 0.0f, 0.0f, 0.0f, 1.0f }))
            {
                GUI::GUI::Instance().Render(Window);
                Renderer->EndRender(Window);
            }
        }
    }

    return true;
}

static bool InitializeResources()
{
    const UniquePtr<Render::Renderer>& Renderer { Render::Renderer::Instance() };

    Render::GraphicsPipelineDescription TestDesc {};
    TestDesc.Name = "Test";
    TestDesc.CullMode = Render::CullModeType::Back;
    TestDesc.UseDepthStencilBuffer = true;
    TestDesc.VertexShader.Name = "DefaultVS";
    TestDesc.VertexShader.Path = "TestVS";
    TestDesc.VertexShader.Function = "Main";
    TestDesc.VertexShader.VertexDescriptions.Push({ "POSITION", Render::VertexFormat::Float3 });
    TestDesc.VertexShader.VertexDescriptions.Push({ "TEXCOORD", Render::VertexFormat::Float2 });
    TestDesc.VertexShader.VertexDescriptions.Push({ "COLOR", Render::VertexFormat::Float4 });
    TestDesc.FragmentShader.Name = "DefaultFS";
    TestDesc.FragmentShader.Path = "TestPS";
    TestDesc.FragmentShader.Function = "Main";
    g_TestPipeline = Renderer->CreateGraphicsPipeline(TestDesc);
    if (!g_TestPipeline.IsValid())
    {
        return false;
    }

    const float Offset { 1.0f };
    Vertex3 Vertices[3];
    Vertices[0] = { { 0.0f, Offset, 5.0f }, { 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } };
    Vertices[1] = { { -Offset, -Offset, 5.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } };
    Vertices[2] = { { Offset, -Offset, 5.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } };
    u32 IndexBufferData[] = { 0, 1, 2 };

    Engine::ECS::EntityID Entity {
        Engine::Engine::Instance().World()->NewEntity<Engine::Components::Transform, Engine::Components::Mesh>()
    };
    Engine::Components::Mesh& Mesh { Engine::Engine::Instance().World()->GetComponent<Engine::Components::Mesh>(
        Entity) };

    Render::VertexBufferDescription MeshBufferDesc {};
    MeshBufferDesc.VertexBufferSize = sizeof(Vertices);
    MeshBufferDesc.Stride = sizeof(Vertex3);
    MeshBufferDesc.IndexBufferSize = sizeof(IndexBufferData);
    MeshBufferDesc.IndexFormat = Render::IndexFormatType::U32;

    Mesh.Indices = ARRAY_COUNT(IndexBufferData);
    Mesh.VertexBuffer = Renderer->CreateVertexBuffer(MeshBufferDesc);
    if (!Mesh.VertexBuffer.IsValid())
    {
        return false;
    }

    Render::VertexDataDescription MeshDataDesc {};
    MeshDataDesc.VertexData = Vertices;
    MeshDataDesc.VertexDataSize = sizeof(Vertices);
    MeshDataDesc.IndexData = IndexBufferData;
    MeshDataDesc.IndexDataSize = sizeof(IndexBufferData);
    if (!Renderer->UploadVertexData(Mesh.VertexBuffer, MeshDataDesc))
    {
        return false;
    }

    const u32 Width { 256 };
    const u32 Height { 256 };
    const Array<u8> Data { GenerateTexture(Width, Height) };
    g_DefaultTexture =
        Renderer->CreateTexture({ const_cast<u8*>(Data.Data()), Width, Height, Render::TextureFormat::RGBAByte });

    if (!GUI::GUI::Instance().InitializeResources())
    {
        return false;
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

    const UniquePtr<Platform::Platform>& Platform { Platform::Platform::Instance() };
    if (!Platform->Initialize())
    {
        printf("Failed to initialize platform!\n");
        return -1;
    }

    if (!Render::Renderer::Instance()->Initialize())
    {
        return -1;
    }

    const Render::Renderer::DriverSummary& Summary { Render::Renderer::Instance()->Summary() };
    Core::Console::WriteLine("Rendering Driver Summary");
    Core::Console::WriteLine("Vendor: %s", Summary.Vendor.Data());
    Core::Console::WriteLine("Renderer: %s", Summary.Renderer.Data());
    Core::Console::WriteLine("Version: %s", Summary.Version.Data());
    Core::Console::WriteLine("Shading Language Version: %s", Summary.ShadingLanguageVersion.Data());

    if (!GUI::GUI::Instance().Initialize(Argc, Argv))
    {
        return -1;
    }

    if (!Engine::Engine::Instance().Initialize())
    {
        Core::Console::Error("Failed to initialize engine!");
        return -1;
    }

    if (!InitializeResources())
    {
        Core::Console::Error("Failed to initialize rendering resources.");
        return -1;
    }

    int Return = Platform->SetOnFrame(OnPlatformFrame).Run();

    Engine::Engine::Instance().Shutdown();
    GUI::GUI::Instance().Shutdown();

    Platform->Shutdown();
    Platform::Debugger::Instance()->Shutdown();

    return Return;
}

}
}

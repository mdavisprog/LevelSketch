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
#include "../Engine/Camera.hpp"
#include "../Engine/Engine.hpp"
#include "../GUI/GUI.hpp"
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

static Engine::Camera g_Camera { { 0.0f, 0.0f, -20.0f } };
static bool g_RotateCamera { false };
static Vector2i g_LastMousePos {};
static Vector2i g_LockedMousePos {};

static void UpdateCamera(f32 DeltaTime)
{
    g_Camera.Update(DeltaTime);
    Render::Renderer::Instance()->UpdateViewMatrix(g_Camera.ToViewMatrix());
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
        g_Camera.SetMovement(Flags);
    }
    else
    {
        g_Camera.ClearMovement(Flags);
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
        if (Event.GetData().MouseButton.Button == Platform::Mouse::Button::Left)
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
            g_Camera.Yaw(static_cast<f32>(MouseDelta.X)).Pitch(static_cast<f32>(-MouseDelta.Y));
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

    UpdateCamera(TimingData.DeltaSeconds);

    GUI.RunFrame();

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

    if (!GUI::GUI::Instance().Initialize(Argc, Argv))
    {
        return false;
    }

    if (!Engine::Engine::Instance().Initialize())
    {
        Core::Console::Error("Failed to initialize engine!");
        return 0;
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

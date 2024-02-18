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

#include "Window.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "../EventQueue.hpp"
#include "SDL2/SDL.h"

namespace LevelSketch
{
namespace Platform
{
namespace SDL2
{

static Mouse::Button::Type ToButton(u8 Button)
{
    switch (Button)
    {
    case SDL_BUTTON_LEFT: return Mouse::Button::Left;
    case SDL_BUTTON_RIGHT: return Mouse::Button::Right;
    case SDL_BUTTON_MIDDLE: return Mouse::Button::Middle;
    default: break;
    }

    return Mouse::Button::None;
}

static Keyboard::Key ToKey(SDL_Scancode Code)
{
    switch (Code)
    {
    case SDL_SCANCODE_A: return Keyboard::Key::A;
    case SDL_SCANCODE_B: return Keyboard::Key::B;
    case SDL_SCANCODE_C: return Keyboard::Key::C;
    case SDL_SCANCODE_D: return Keyboard::Key::D;
    case SDL_SCANCODE_E: return Keyboard::Key::E;
    case SDL_SCANCODE_F: return Keyboard::Key::F;
    case SDL_SCANCODE_G: return Keyboard::Key::G;
    case SDL_SCANCODE_H: return Keyboard::Key::H;
    case SDL_SCANCODE_I: return Keyboard::Key::I;
    case SDL_SCANCODE_J: return Keyboard::Key::J;
    case SDL_SCANCODE_K: return Keyboard::Key::K;
    case SDL_SCANCODE_L: return Keyboard::Key::L;
    case SDL_SCANCODE_M: return Keyboard::Key::M;
    case SDL_SCANCODE_N: return Keyboard::Key::N;
    case SDL_SCANCODE_O: return Keyboard::Key::O;
    case SDL_SCANCODE_P: return Keyboard::Key::P;
    case SDL_SCANCODE_Q: return Keyboard::Key::Q;
    case SDL_SCANCODE_R: return Keyboard::Key::R;
    case SDL_SCANCODE_S: return Keyboard::Key::S;
    case SDL_SCANCODE_T: return Keyboard::Key::T;
    case SDL_SCANCODE_U: return Keyboard::Key::U;
    case SDL_SCANCODE_V: return Keyboard::Key::V;
    case SDL_SCANCODE_W: return Keyboard::Key::W;
    case SDL_SCANCODE_X: return Keyboard::Key::X;
    case SDL_SCANCODE_Y: return Keyboard::Key::Y;
    case SDL_SCANCODE_Z: return Keyboard::Key::Z;
    case SDL_SCANCODE_1: return Keyboard::Key::One;
    case SDL_SCANCODE_2: return Keyboard::Key::Two;
    case SDL_SCANCODE_3: return Keyboard::Key::Three;
    case SDL_SCANCODE_4: return Keyboard::Key::Four;
    case SDL_SCANCODE_5: return Keyboard::Key::Five;
    case SDL_SCANCODE_6: return Keyboard::Key::Six;
    case SDL_SCANCODE_7: return Keyboard::Key::Seven;
    case SDL_SCANCODE_8: return Keyboard::Key::Eight;
    case SDL_SCANCODE_9: return Keyboard::Key::Nine;
    case SDL_SCANCODE_0: return Keyboard::Key::Zero;
    default: break;
    }

    return Keyboard::Key::None;
}

Window::Window()
    : LevelSketch::Platform::Window()
{
}

void* Window::Handle() const
{
    return reinterpret_cast<void*>(m_Handle);
}

bool Window::Create(const char* Title, int X, int Y, int Width, int Height)
{
    if (IsOpen())
    {
        return true;
    }

    u32 Flags = SDL_WINDOW_ALLOW_HIGHDPI;

#if defined(RENDER_OPENGL)
    Flags |= SDL_WINDOW_OPENGL;
#elif defined(RENDER_VULKAN)
    Flags |= SDL_WINDOW_VULKAN;
#endif

    m_Handle = SDL_CreateWindow(Title, X, Y, Width, Height, Flags);

    if (m_Handle == nullptr)
    {
        Core::Console::Error("Failed to create SDL window: %s", SDL_GetError());
        return false;
    }

    return true;
}

void Window::Close()
{
    if (IsOpen())
    {
        SDL_DestroyWindow(m_Handle);
        m_Handle = nullptr;
    }
}

void Window::Show()
{
    if (IsOpen())
    {
        SDL_ShowWindow(m_Handle);
    }
}

void Window::Focus()
{
    if (IsOpen())
    {
        SDL_RaiseWindow(m_Handle);
    }
}

void Window::SetPosition(int X, int Y)
{
    if (IsOpen())
    {
        SDL_SetWindowPosition(m_Handle, X, Y);
    }
}

Core::Math::Vector2i Window::Position() const
{
    if (!IsOpen())
    {
        return { 0, 0 };
    }

    Core::Math::Vector2i Result;
    SDL_GetWindowPosition(m_Handle, &Result.X, &Result.Y);

    return Result;
}

Core::Math::Vector2i Window::Size() const
{
    if (!IsOpen())
    {
        return { 0, 0 };
    }

    Core::Math::Vector2i Result;
    SDL_GetWindowSize(m_Handle, &Result.X, &Result.Y);

    return Result;
}

void Window::ProcessEvents()
{
    SDL_Event Event;
    while (SDL_PollEvent(&Event))
    {
        switch (Event.type)
        {
        case SDL_WINDOWEVENT: ProcessEvent(Event.window); break;

        case SDL_MOUSEMOTION:
        {
            Event::OnMouseMove MouseMove { { Event.motion.x, Event.motion.y } };
            EventQueue::Instance().Push(MouseMove, this);
        }
        break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            const bool Pressed { Event.type == SDL_MOUSEBUTTONDOWN };
            const Vector2i Position { Event.button.x, Event.button.y };
            Event::OnMouseButton MouseButton { ToButton(Event.button.button), Pressed, Position };
            EventQueue::Instance().Push(MouseButton, this);
        }
        break;

        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            const bool Pressed { Event.type == SDL_KEYDOWN };
            Event::OnKey Key { ToKey(Event.key.keysym.scancode), Pressed };
            EventQueue::Instance().Push(Key, this);
        }
        break;

        default: break;
        }
    }
}

bool Window::IsOpen() const
{
    return m_Handle != nullptr;
}

Window& Window::ProcessEvent(const SDL_WindowEvent& Event)
{
    switch (Event.event)
    {
    case SDL_WINDOWEVENT_CLOSE: Close(); break;

    default: break;
    }

    return *this;
}

}
}
}

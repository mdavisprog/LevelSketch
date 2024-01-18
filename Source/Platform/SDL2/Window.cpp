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

#include "Window.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "SDL2/SDL.h"

namespace LevelSketch
{
namespace Platform
{
namespace SDL2
{

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
        case SDL_WINDOWEVENT:
            ProcessEvent(Event.window);
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
    case SDL_WINDOWEVENT_CLOSE:
        Close();
        break;

    default: break;
    }

    return *this;
}

}
}
}

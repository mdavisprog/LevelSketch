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

#include "Platform.hpp"
#include "../Core/Memory/UniquePtr.hpp"
#include "Window.hpp"

#if defined(PLATFORM_WINDOWS)
    #include "Windows/Platform.hpp"
#elif defined(PLATFORM_MAC)
    #include "Mac/Platform.hpp"
#elif defined(PLATFORM_SDL2)
    #include "SDL2/Platform.hpp"
#else
    #error "Platform is not supported!"
#endif

#include <utility>

namespace LevelSketch
{
namespace Platform
{

const Core::Memory::UniquePtr<Platform>& Platform::Instance()
{
    static Core::Memory::UniquePtr<Platform> Instance
    {
#if defined(PLATFORM_WINDOWS)
        Core::Memory::UniquePtr<Windows::Platform>::New()
#elif defined(PLATFORM_MAC)
        Core::Memory::UniquePtr<Mac::Platform>::New()
#elif defined(PLATFORM_SDL2)
        Core::Memory::UniquePtr<SDL2::Platform>::New()
#endif
    };

    return Instance;
}

int Platform::Run()
{
    return -1;
}

bool Platform::UseCustomLoop() const
{
    return false;
}

Platform& Platform::SetOnFrame(OnFrameSignature&& Fn)
{
    m_OnFrame = std::move(Fn);
    return *this;
}

const Core::Memory::UniquePtr<Window>& Platform::NewWindow(const char* Title, i32 X, i32 Y, i32 Width, i32 Height)
{
    Core::Memory::UniquePtr<Window> Result { InternalNewWindow() };

    if (Result == nullptr)
    {
        return nullptr;
    }

    if (!Result->Create(Title, X, Y, Width, Height))
    {
        return nullptr;
    }

    m_Windows.Push(std::move(Result));

    return m_Windows.Back();
}

Platform& Platform::CloseWindow(Window* Window)
{
    if (Window == nullptr)
    {
        return *this;
    }

    Window->Close();
    
    for (const Core::Memory::UniquePtr<LevelSketch::Platform::Window>& Item : m_Windows)
    {
        if (Item == Window)
        {
            m_Windows.Remove(Item);
            break;
        }
    }

    return *this;
}

}
}

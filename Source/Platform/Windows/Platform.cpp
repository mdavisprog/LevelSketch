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
#include "Events.hpp"
#include "Window.hpp"

#include <cstdio>

namespace LevelSketch
{
namespace Platform
{
namespace Windows
{

Platform::Platform()
    : LevelSketch::Platform::Platform()
{
}

bool Platform::Initialize()
{
    WNDCLASSEXW Class;
    ZeroMemory(&Class, sizeof(WNDCLASSEXW));
    Class.cbSize = sizeof(Class);
    Class.style = CS_HREDRAW | CS_VREDRAW;
    Class.lpszClassName = WND_CLASS_NAME;
    Class.lpfnWndProc = Event::WndProc;
    Class.hInstance = GetModuleHandleW(nullptr);

    if (RegisterClassExW(&Class) == 0)
    {
        printf("Failed to register Window class!\n");
        return false;
    }

    return true;
}

void Platform::Shutdown()
{
    UnregisterClassW(WND_CLASS_NAME, nullptr);
}

const char* Platform::Name() const
{
    return "Windows";
}

Core::Memory::UniquePtr<LevelSketch::Platform::Window> Platform::InternalNewWindow() const
{
    return Core::Memory::UniquePtr<Window>::New();
}

}
}
}

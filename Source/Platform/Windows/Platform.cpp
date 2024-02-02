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

#include "Platform.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Math/Math.hpp"
#include "Errors.hpp"
#include "Events.hpp"
#include "Window.hpp"

#include <cstdio>
#include <hidusage.h>

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
    WNDCLASSEXW Class { 0 };
    Class.cbSize = sizeof(Class);
    Class.style = CS_HREDRAW | CS_VREDRAW;
    Class.lpszClassName = WND_CLASS_NAME;
    Class.lpfnWndProc = Event::WndProc;
    Class.hInstance = GetModuleHandleW(nullptr);
    Class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    Class.hIcon = nullptr;
    Class.hIconSm = nullptr;
    Class.hbrBackground = nullptr;
    Class.lpszMenuName = nullptr;

    if (RegisterClassExW(&Class) == 0)
    {
        Core::Console::Error("Failed to register Window class!");
        return false;
    }

    QueryPerformanceFrequency(&m_Frequency);
    QueryPerformanceCounter(&m_LastTime);

    m_MaxDeltaTicks = static_cast<u64>(m_Frequency.QuadPart / 10);

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

UniquePtr<LevelSketch::Platform::Window> Platform::InternalNewWindow() const
{
    return UniquePtr<Window>::New();
}

void Platform::UpdateTimingData(TimingData& Data)
{
    LARGE_INTEGER Current;
    QueryPerformanceCounter(&Current);

    const u64 DeltaTicks = Core::Math::Min(static_cast<u64>(Current.QuadPart - m_LastTime.QuadPart), m_MaxDeltaTicks);
    m_LastTime = Current;
    Data.DeltaSeconds = static_cast<float>(DeltaTicks) / static_cast<float>(m_Frequency.QuadPart);
}

bool Platform::RegisterRawInputDevice(HWND Target)
{
    if (m_RegisteredDevice)
    {
        return true;
    }

    RAWINPUTDEVICE Rid[1];
    Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    Rid[0].dwFlags = RIDEV_INPUTSINK;
    Rid[0].hwndTarget = Target;

    if (!RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])))
    {
        const DWORD Error { GetLastError() };
        Core::Console::Error("Failed to register raw input device. Error: %s", Errors::ToString(Error).Data());
        return false;
    }

    m_RegisteredDevice = true;
    return true;
}

}
}
}

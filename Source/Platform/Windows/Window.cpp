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
#include "../../Core/Math/Vector2.hpp"
#include "../WindowDescription.hpp"
#include "Platform.hpp"

#include <string>

namespace LevelSketch
{
namespace Platform
{
namespace Windows
{

Window::Window()
    : LevelSketch::Platform::Window()
{
}

void* Window::Handle() const
{
    return reinterpret_cast<void*>(m_Handle);
}

bool Window::Create(const WindowDescription& Description)
{
    if (m_Handle != nullptr)
    {
        return true;
    }

    const i32 TitleLength { static_cast<i32>(Description.Title.Length()) };
    const i32 Length { MultiByteToWideChar(CP_ACP, 0, Description.Title.Data(), TitleLength, nullptr, 0) };
    std::wstring wTitle;
    wTitle.resize(Length);
    MultiByteToWideChar(CP_ACP, 0, Description.Title.Data(), TitleLength, wTitle.data(), Length);

    DWORD Style { WS_OVERLAPPEDWINDOW };

    if (!Description.CanMinimize)
    {
        Style &= ~WS_MINIMIZEBOX;
    }

    if (Description.Maximized)
    {
        Style |= WS_MAXIMIZE;
    }

    m_Handle = CreateWindowExW(0,
        WND_CLASS_NAME,
        wTitle.data(),
        Style,
        Description.Position.X,
        Description.Position.Y,
        Description.Size.X,
        Description.Size.Y,
        nullptr,
        nullptr,
        nullptr,
        nullptr);

    if (IsOpen())
    {
        SetWindowLongPtrW(m_Handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }

    Platform* Platform_ { static_cast<Platform*>(Platform::Platform::Instance().Get()) };
    if (!Platform_->RegisterRawInputDevice(m_Handle))
    {
        Close();
        return false;
    }

    return IsOpen();
}

void Window::Close()
{
    if (m_Handle == nullptr)
    {
        return;
    }

    DestroyWindow(m_Handle);
    m_Handle = nullptr;
}

void Window::Show()
{
    if (m_Handle == nullptr)
    {
        return;
    }

    ShowWindow(m_Handle, SW_SHOW);
}

void Window::Focus()
{
    if (m_Handle == nullptr)
    {
        return;
    }

    SetForegroundWindow(m_Handle);
}

void Window::SetPosition(int X, int Y)
{
    if (m_Handle == nullptr)
    {
        return;
    }

    SetWindowPos(m_Handle, HWND_TOP, X, Y, 0, 0, SWP_NOSIZE);
}

Core::Math::Vector2i Window::Position() const
{
    if (!IsOpen())
    {
        return {};
    }

    RECT Rect;
    GetWindowRect(m_Handle, &Rect);

    return { Rect.left, Rect.top };
}

Core::Math::Vector2i Window::Size() const
{
    if (!IsOpen())
    {
        return {};
    }

    RECT Rect;
    GetWindowRect(m_Handle, &Rect);

    return { Rect.right - Rect.left, Rect.bottom - Rect.top };
}

void Window::ProcessEvents()
{
    if (m_Handle == nullptr)
    {
        return;
    }

    MSG Msg {};
    while (PeekMessageW(&Msg, m_Handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Msg);
        DispatchMessageW(&Msg);
    }
}

bool Window::IsOpen() const
{
    return m_Handle != nullptr;
}

}
}
}

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

#include "Events.hpp"
#include "../EventQueue.hpp"
#include "../Window.hpp"

#include <windowsx.h>

namespace LevelSketch
{
namespace Platform
{
namespace Windows
{

static Vector2i Position(LPARAM Param)
{
    return { GET_X_LPARAM(Param), GET_Y_LPARAM(Param) };
}

LRESULT Event::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LevelSketch::Platform::Window* Window { reinterpret_cast<LevelSketch::Platform::Window*>(
        GetWindowLongPtrW(hWnd, GWLP_USERDATA)) };

    switch (Msg)
    {
    case WM_CLOSE: Window->Close(); break;

    case WM_DESTROY: PostQuitMessage(0); break;

    case WM_MOUSEMOVE:
    {
        Platform::Event::OnMouseMove MouseMove { Position(lParam) };
        EventQueue::Instance().Push({ MouseMove }, Window);
    }
    break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    {
        Mouse::Button::Type Button { Mouse::Button::Left };
        switch (Msg)
        {
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP: Button = Mouse::Button::Middle; break;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP: Button = Mouse::Button::Right; break;
        default: break;
        }

        const bool Pressed { Msg == WM_LBUTTONDOWN || Msg == WM_RBUTTONDOWN || Msg == WM_MBUTTONDOWN };
        Platform::Event::OnMouseButton MouseButton { Button, Pressed, Position(lParam) };
        EventQueue::Instance().Push({ MouseButton }, Window);
    }
    break;

    default: break;
    }

    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

}
}
}

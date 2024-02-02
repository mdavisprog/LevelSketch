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

Vector2i Event::s_MousePosition {};

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
        if (Mouse::GetMoveMode() == Mouse::MoveMode::Absolute)
        {
            s_MousePosition = Position(lParam);
            Platform::Event::OnMouseMove MouseMove { s_MousePosition };
            EventQueue::Instance().Push(MouseMove, Window);
        }
    }
    break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    {
        s_MousePosition = Position(lParam);

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
        Platform::Event::OnMouseButton MouseButton { Button, Pressed, s_MousePosition };
        EventQueue::Instance().Push(MouseButton, Window);
    }
    break;

    case WM_INPUT:
    {
        if (Mouse::GetMoveMode() == Mouse::MoveMode::Relative)
        {
            RAWINPUT RawInput {};
            u32 Size { sizeof(RawInput) };

            const u32 Result { GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
                RID_INPUT,
                &RawInput,
                &Size,
                sizeof(RAWINPUTHEADER)) };
            if (Result != static_cast<u32>(-1))
            {
                if (RawInput.header.dwType == RIM_TYPEMOUSE)
                {
                    if (!(RawInput.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE))
                    {
                        s_MousePosition.X += RawInput.data.mouse.lLastX;
                        s_MousePosition.Y += RawInput.data.mouse.lLastY;
                        Platform::Event::OnMouseMove MouseMove { s_MousePosition };
                        EventQueue::Instance().Push(MouseMove, Window);
                    }
                }
            }
        }
    }
    break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        Keyboard::Key Key { Keyboard::Key::None };
        switch (wParam)
        {
        case 0x30: Key = Keyboard::Key::Zero; break;
        case 0x31: Key = Keyboard::Key::One; break;
        case 0x32: Key = Keyboard::Key::Two; break;
        case 0x33: Key = Keyboard::Key::Three; break;
        case 0x34: Key = Keyboard::Key::Four; break;
        case 0x35: Key = Keyboard::Key::Five; break;
        case 0x36: Key = Keyboard::Key::Six; break;
        case 0x37: Key = Keyboard::Key::Seven; break;
        case 0x38: Key = Keyboard::Key::Eight; break;
        case 0x39: Key = Keyboard::Key::Nine; break;
        case 0x41: Key = Keyboard::Key::A; break;
        case 0x42: Key = Keyboard::Key::B; break;
        case 0x43: Key = Keyboard::Key::C; break;
        case 0x44: Key = Keyboard::Key::D; break;
        case 0x45: Key = Keyboard::Key::E; break;
        case 0x46: Key = Keyboard::Key::F; break;
        case 0x47: Key = Keyboard::Key::G; break;
        case 0x48: Key = Keyboard::Key::H; break;
        case 0x49: Key = Keyboard::Key::I; break;
        case 0x4A: Key = Keyboard::Key::J; break;
        case 0x4B: Key = Keyboard::Key::K; break;
        case 0x4C: Key = Keyboard::Key::L; break;
        case 0x4D: Key = Keyboard::Key::M; break;
        case 0x4E: Key = Keyboard::Key::N; break;
        case 0x4F: Key = Keyboard::Key::O; break;
        case 0x50: Key = Keyboard::Key::P; break;
        case 0x51: Key = Keyboard::Key::Q; break;
        case 0x52: Key = Keyboard::Key::R; break;
        case 0x53: Key = Keyboard::Key::S; break;
        case 0x54: Key = Keyboard::Key::T; break;
        case 0x55: Key = Keyboard::Key::U; break;
        case 0x56: Key = Keyboard::Key::V; break;
        case 0x57: Key = Keyboard::Key::W; break;
        case 0x58: Key = Keyboard::Key::X; break;
        case 0x59: Key = Keyboard::Key::Y; break;
        case 0x5A: Key = Keyboard::Key::Z; break;
        default: break;
        }

        const bool Pressed { Msg == WM_KEYDOWN };
        const Platform::Event::OnKey OnKey { Key, Pressed };
        EventQueue::Instance().Push(OnKey, Window);
    }
    break;

    default: break;
    }

    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

}
}
}

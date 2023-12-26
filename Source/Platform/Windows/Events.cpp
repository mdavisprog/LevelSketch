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

LRESULT Event::WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    LevelSketch::Platform::Window* Window { reinterpret_cast<LevelSketch::Platform::Window*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)) };

    switch (Msg)
    {
    case WM_CLOSE:
        Window->Close();
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    
    case WM_MOUSEMOVE:
    {
        Vector2i Position { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        Platform::Event::OnMouseMove MouseMove { Position };
        EventQueue::Instance().Push({ MouseMove }, Window);
    } break;

    default: break;
    }

    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

}
}
}

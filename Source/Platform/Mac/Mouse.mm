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

#include "../Mouse.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "Window.hpp"
#include "WindowBridge.hpp"

#import <AppKit/AppKit.h>

namespace LevelSketch
{
namespace Platform
{

static bool g_CursorVisible { true };

void Mouse::Show()
{
    if (!IsVisible())
    {
        [NSCursor unhide];
        g_CursorVisible = true;
    }
}

void Mouse::Hide()
{
    if (IsVisible())
    {
        [NSCursor hide];
        g_CursorVisible = false;
    }
}

bool Mouse::IsVisible()
{
    return g_CursorVisible;
}

void Mouse::SetPosition(const Vector2i& Position)
{
    CGEventSourceRef EventSource { CGEventSourceCreate(kCGEventSourceStateCombinedSessionState) };
    CGEventSourceSetLocalEventsSuppressionInterval(EventSource, 0.0);
    CGAssociateMouseAndMouseCursorPosition(0);
    CGWarpMouseCursorPosition({ static_cast<f32>(Position.X), static_cast<f32>(Position.Y) });
    CGAssociateMouseAndMouseCursorPosition(1);
    CFRelease(EventSource);
}

void Mouse::SetPosition(Window* Target, const Vector2i& Position)
{
    @autoreleasepool
    {
        WindowBridge* Bridge { [WindowBridge Retrieve:Target->Handle()] };
        NSPoint Point { static_cast<f32>(Position.X), static_cast<f32>(Position.Y) };
        Point.y = Bridge.Window.contentView.frame.size.height - Point.y;
        Point = [Bridge.Window convertPointToScreen:Point];
        Point.y = Bridge.Window.screen.frame.size.height - Point.y;
        SetPosition({ static_cast<i32>(Point.x), static_cast<i32>(Point.y) });
    }
}

}
}

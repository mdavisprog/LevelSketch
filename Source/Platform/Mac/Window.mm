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
#include "../../Core/Assert.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "Platform.hpp"
#include "WindowBridge.hpp"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

namespace LevelSketch
{
namespace Platform
{
namespace Mac
{

Window::Window()
    : LevelSketch::Platform::Window()
{
}

void* Window::Handle() const
{
    return m_Bridge;
}

bool Window::Create(const char* Title, int X, int Y, int Width, int Height)
{
    if (m_Bridge != nullptr)
    {
        return true;
    }

    @autoreleasepool
    {
        WindowBridge* Bridge = [WindowBridge alloc];

        Bridge.Window = [[NSWindow alloc]
            initWithContentRect:NSMakeRect(X, Y, Width, Height)
            styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable)
            backing:NSBackingStoreBuffered
            defer:NO
        ];

        [Bridge.Window setTitle:[NSString stringWithUTF8String:Title]];

        m_Bridge = (void*)CFBridgingRetain(Bridge);
    }

    return true;
}

void Window::Close()
{
    if (!IsOpen())
    {
        return;
    }

    @autoreleasepool
    {
        WindowBridge* Bridge = [WindowBridge Retrieve:m_Bridge];
        [Bridge.Window close];
        m_Bridge = nullptr;
    }
}

void Window::Show()
{
    if (!IsOpen())
    {
        return;
    }

    @autoreleasepool
    {
        WindowBridge* Bridge = [WindowBridge Retrieve:m_Bridge];
        [Bridge.Window makeKeyAndOrderFront:nil];
    }
}

void Window::Focus()
{
    if (!IsOpen())
    {
        return;
    }

    @autoreleasepool
    {
        WindowBridge* Bridge = [WindowBridge Retrieve:m_Bridge];
        [Bridge.Window orderFront:nil];
    }
}

void Window::SetPosition(int X, int Y)
{
    if (!IsOpen())
    {
        return;
    }

    @autoreleasepool
    {
        WindowBridge* Bridge = [WindowBridge Retrieve:m_Bridge];

        // Can be nil if window is off-screen.
        if (Bridge.Window.screen != nil)
        {
            Y = (int)Bridge.Window.screen.frame.size.height - (int)Bridge.Window.frame.size.height - Y;
        }
        else
        {
            Y = Bridge.Window.frame.size.height - Y;
        }

        [Bridge.Window setFrameOrigin:NSMakePoint(X, Y)];
    }
}

Core::Math::Vector2i Window::Position() const
{
    Core::Math::Vector2i Result;

    if (!IsOpen())
    {
        return Result;
    }

    @autoreleasepool
    {
        // TODO: Properly check for screen.
        WindowBridge* Bridge = [WindowBridge Retrieve:m_Bridge];
        NSRect Frame = [Bridge.Window frame];
        Result.X = Frame.origin.x;
        Result.Y = Frame.origin.y + Frame.size.height;
    }

    return Result;
}

Core::Math::Vector2i Window::Size() const
{
    Core::Math::Vector2i Result;

    if (!IsOpen())
    {
        return Result;
    }

    @autoreleasepool
    {
        WindowBridge* Bridge = [WindowBridge Retrieve:m_Bridge];
        NSRect Frame = [Bridge.Window frame];
        Result.X = Frame.size.width;
        Result.Y = Frame.size.height;
    }

    return Result;
}

void Window::ProcessEvents()
{
}

bool Window::IsOpen() const
{
    return m_Bridge != nullptr;
}

}
}
}

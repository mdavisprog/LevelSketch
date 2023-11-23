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
#include "../../Core/Math/Vector2.hpp"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

@interface WindowProxy : NSObject
    @property (strong) NSWindow* Window;
@end

@implementation WindowProxy
@end

namespace LevelSketch
{
namespace Platform
{
namespace Mac
{

WindowProxy* GetWindowProxy(void* Ptr)
{
    return (__bridge WindowProxy*)Ptr;
}

Window::Window()
    : LevelSketch::Platform::Window()
{
}

void* Window::Handle() const
{
    return nullptr;
}

bool Window::Create(const char* Title, int X, int Y, int Width, int Height)
{
    if (m_WindowProxy != nullptr)
    {
        return true;
    }

    @autoreleasepool
    {
        WindowProxy* Proxy = [WindowProxy alloc];

        Proxy.Window = [[NSWindow alloc]
            initWithContentRect:NSMakeRect(X, Y, Width, Height)
            styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable)
            backing:NSBackingStoreBuffered
            defer:NO
        ];

        [Proxy.Window setTitle:[NSString stringWithUTF8String:Title]];

        m_WindowProxy = (void*)CFBridgingRetain(Proxy);
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
        WindowProxy* Proxy = GetWindowProxy(m_WindowProxy);
        [Proxy.Window close];
        [Proxy dealloc];
        m_WindowProxy = nullptr;
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
        WindowProxy* Proxy = GetWindowProxy(m_WindowProxy);
        [Proxy.Window makeKeyAndOrderFront:nil];
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
        WindowProxy* Proxy = GetWindowProxy(m_WindowProxy);
        [Proxy.Window orderFront:nil];
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
        WindowProxy* Proxy = GetWindowProxy(m_WindowProxy);

        // Can be nil if window is off-screen.
        if (Proxy.Window.screen != nil)
        {
            Y = (int)Proxy.Window.screen.frame.size.height - (int)Proxy.Window.frame.size.height - Y;
        }
        else
        {
            Y = Proxy.Window.frame.size.height - Y;
        }

        [Proxy.Window setFrameOrigin:NSMakePoint(X, Y)];
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
        WindowProxy* Proxy = GetWindowProxy(m_WindowProxy);
        NSRect Frame = [Proxy.Window frame];
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
        WindowProxy* Proxy = GetWindowProxy(m_WindowProxy);
        NSRect Frame = [Proxy.Window frame];
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
    return m_WindowProxy != nullptr;
}

}
}
}

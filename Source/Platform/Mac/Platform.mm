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
#include "Window.hpp"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

@interface AppDelegate : NSObject<NSApplicationDelegate>
@end

@implementation AppDelegate

    -(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
    {
        return YES;
    }

    -(void)applicationDidFinishLaunching:(NSNotification*)Notification
    {
        [NSApp activateIgnoringOtherApps:YES];
    }

@end

namespace LevelSketch
{
namespace Platform
{
namespace Mac
{

Platform::Platform()
    : LevelSketch::Platform::Platform()
{
}

bool Platform::Initialize()
{
    @autoreleasepool
    {
        AppDelegate* Delegate = [[AppDelegate alloc] init];
        NSApplication* Application = [NSApplication sharedApplication];
        [Application setDelegate:Delegate];
        [Application setActivationPolicy:NSApplicationActivationPolicyRegular];
    }

    return true;
}

void Platform::Shutdown()
{
}

const char* Platform::Name() const
{
    return "Mac";
}

int Platform::Run()
{
    [NSApp run];
    return 0;
}

Core::Memory::UniquePtr<LevelSketch::Platform::Window> Platform::InternalNewWindow() const
{
    return Core::Memory::UniquePtr<Window>::New();
}

void Platform::UpdateTimingData(TimingData& Data)
{
    CFTimeInterval Current { (double)clock_gettime_nsec_np(CLOCK_UPTIME_RAW) / 1e9 };
    Data.DeltaSeconds = (float)(Current - m_LastTime);
    m_LastTime = Current;
}

}
}
}

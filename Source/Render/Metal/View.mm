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

#include "View.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/Mac/Platform.hpp"
#include "../../Platform/Window.hpp"

#import <CoreVideo/CoreVideo.h>

@implementation View
{
    CVDisplayLinkRef m_DisplayLink;
}

-(void) viewDidMoveToWindow
{
    @autoreleasepool
    {
        [super viewDidMoveToWindow];

        // TODO: Check if callback has already been setup by the main display.

        CVReturn Return = CVDisplayLinkCreateWithActiveCGDisplays(&m_DisplayLink);
        if (Return != kCVReturnSuccess)
        {
            LevelSketch::Core::Console::Error("Failed to create display link.");
            return;
        }

        Return = CVDisplayLinkSetOutputCallback(
            m_DisplayLink,
            &LevelSketch::Platform::Mac::Platform::OnDisplayLinkOutput,
            nullptr);

        if (Return != kCVReturnSuccess)
        {
            LevelSketch::Core::Console::Error("Failed to display link output callback.");
            return;
        }

        CVDisplayLinkStart(m_DisplayLink);

        NSNotificationCenter* NotificationCenter = [NSNotificationCenter defaultCenter];
        [NotificationCenter addObserver:self
                               selector:@selector(windowWillClose:)
                                   name:NSWindowWillCloseNotification
                                 object:self.window];
    }
}

-(void) windowWillClose:(NSNotification*)Notification
{
    @autoreleasepool
    {
        if (Notification.object == self.window)
        {
            CVDisplayLinkStop(m_DisplayLink);
        }
    }
}

@end

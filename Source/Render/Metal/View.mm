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

#include "View.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/Mac/DisplayLink.hpp"

#import <CoreVideo/CoreVideo.h>

using DisplayLink = LevelSketch::Platform::Mac::DisplayLink;

@implementation View
{
}

- (instancetype)initWithFrame:(CGRect)Frame
{
    self = [super initWithFrame:Frame];

    if (self)
    {
        self.wantsLayer = YES;
        self.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;
        _MetalLayer = (CAMetalLayer*)self.layer;
        self.layer.delegate = self;
    }

    return self;
}

- (CALayer*)makeBackingLayer
{
    return [CAMetalLayer layer];
}

- (void)viewDidMoveToWindow
{
    @autoreleasepool
    {
        [super viewDidMoveToWindow];

        if (!DisplayLink::DisplayLink::Instance().Initialized())
        {
            DisplayLink::DisplayLink::Instance().Initialize();

            NSNotificationCenter* NotificationCenter = [NSNotificationCenter defaultCenter];
            [NotificationCenter addObserver:self
                                   selector:@selector(windowWillClose:)
                                       name:NSWindowWillCloseNotification
                                     object:self.window];
        }
    }
}

- (void)windowWillClose:(NSNotification*)Notification
{
    @autoreleasepool
    {
        if (Notification.object == self.window)
        {
            DisplayLink::DisplayLink::Instance().Shutdown();
        }
    }
}

@end

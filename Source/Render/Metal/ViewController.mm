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

#include "ViewController.hpp"
#include "../../Core/Assert.hpp"
#include "../../Platform/Platform.hpp"
#include "../../Platform/Window.hpp"

@implementation ViewController
{
    id<MTLDevice> m_Device;
    id<MTLCommandQueue> m_CommandQueue;
}

-(instancetype)initWithNibName:(nullable NSString *)nibNameOrNil bundle:(nullable NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle: nibBundleOrNil];

    m_Device = MTLCreateSystemDefaultDevice();
    LS_ASSERT(m_Device != nullptr);

    m_CommandQueue = [m_Device newCommandQueue];

    return self;
}

-(void)loadView
{
    self.view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0, 960, 540)];
}

-(void)viewDidLoad
{
    [super viewDidLoad];

    MTKView* View = (MTKView*)self.view;
    View.device = m_Device;
    View.delegate = self;

    [NSApp activateIgnoringOtherApps:YES];
}

-(void)viewWillAppear
{
    [super viewWillAppear];
    self.view.window.delegate = self;
}

-(void)windowWillClose:(NSNotification*)Notification
{
}

-(void) drawInMTKView:(nonnull MTKView*)View
{
    LevelSketch::Platform::Platform::Instance()->RunFrame();
}

-(void) mtkView:(nonnull MTKView*)View drawableSizeWillChange:(CGSize)Size
{
}

@end

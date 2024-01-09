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

#include "Renderer.hpp"
#include "../../Platform/Window.hpp"
#include "../../Platform/Mac/WindowBridge.hpp"
#include "RenderBridge.hpp"
#include "ViewController.hpp"

#import <AppKit/AppKit.h>

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

Renderer::Renderer()
    : LevelSketch::Render::Renderer()
{
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize()
{
    return true;
}

bool Renderer::Initialize(Platform::Window* Window)
{
    @autoreleasepool
    {
        if (m_RenderBridge == nullptr)
        {
            m_RenderBridge = UniquePtr<RenderBridge>::New();

            WindowBridge* Bridge = [WindowBridge Retrieve:Window->Handle()];
            NSViewController* Root = [[ViewController alloc] initWithNibName:nil bundle:nil];
            Bridge.Window.contentViewController = Root;
            CAMetalLayer* Layer = (CAMetalLayer*)Bridge.Window.contentView.layer;
            
            if (!m_RenderBridge->Initialize(Layer, Window))
            {
                return false;
            }
        }
    }

    return true;
}

void Renderer::Shutdown()
{
}

void Renderer::Render(Platform::Window* Window)
{
    @autoreleasepool
    {
        WindowBridge* Bridge = [WindowBridge Retrieve:Window->Handle()];
        CAMetalLayer* Layer = (CAMetalLayer*)Bridge.Window.contentView.layer;

        @synchronized(Layer)
        {
            m_RenderBridge->Render(Layer, Window);
        }
    }
}

u32 Renderer::LoadTexture(const void*, u32, u32, u8)
{
    return 1;
}

}
}
}

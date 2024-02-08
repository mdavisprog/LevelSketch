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
#include "../../Core/Math/Vector2.hpp"
#include "../../Platform/FileSystem.hpp"
#include "../../Platform/Mac/WindowBridge.hpp"
#include "../../Platform/Window.hpp"
#include "CommandBuffer.hpp"
#include "CommandEncoder.hpp"
#include "CommandQueue.hpp"
#include "DepthStencil.hpp"
#include "Device.hpp"
#include "Texture.hpp"
#include "ViewController.hpp"

#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace LevelSketch
{
namespace Render
{

String Renderer::ShadersDirectory()
{
    return Platform::FileSystem::CombinePaths(Platform::FileSystem::ContentDirectory(), "Metal");
}

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
    m_Device = UniquePtr<Device>::New();

    if (!m_Device->Initialize())
    {
        return false;
    }

    m_DepthStencil = UniquePtr<DepthStencil>::New();
    if (!m_DepthStencil->Initialize(m_Device.Get()))
    {
        return false;
    }

    return true;
}

bool Renderer::Initialize(Platform::Window* Window)
{
    @autoreleasepool
    {
        WindowBridge* Bridge = [WindowBridge Retrieve:Window->Handle()];
        NSViewController* Root = [[ViewController alloc] initWithNibName:nil bundle:nil];
        Bridge.Window.contentViewController = Root;
        [Bridge OnViewCreated:Bridge.Window.contentView Window:Window];
    }

    return true;
}

void Renderer::Shutdown()
{
    m_Textures.Clear();
}

u32 Renderer::LoadTexture(const void* Data, u32 Width, u32 Height, u8)
{
    UniquePtr<Texture> Texture_ { UniquePtr<Texture>::New() };

    if (!Texture_->Initialize(m_Device.Get(), Width, Height))
    {
        return 0;
    }

    if (!Texture_->Upload(Data))
    {
        return 0;
    }

    const u32 Result { Texture_->ID() };
    m_Textures.Push(std::move(Texture_));
    return Result;
}

bool Renderer::BindTexture(u32 ID)
{
    Texture* Target { GetTexture(ID) };

    if (Target == nullptr)
    {
        return false;
    }

    @autoreleasepool
    {
        CommandEncoder* Encoder { m_Device->GetCommandQueue()->CurrentBuffer()->CurrentEncoder() };
        [Encoder->Get() setFragmentTexture:Target->Data() atIndex:0];
    }

    return true;
}

bool Renderer::BeginRender(Platform::Window* Window, const Colorf& ClearColor)
{
    @autoreleasepool
    {
        WindowBridge* Bridge = [WindowBridge Retrieve:Window->Handle()];
        CAMetalLayer* Layer { static_cast<CAMetalLayer*>(Bridge.Window.contentView.layer) };

        const CGSize Size { Bridge.Window.frame.size };
        const CGFloat Scale { Bridge.Window.screen.backingScaleFactor };
        Layer.drawableSize = { Size.width * Scale, Size.height * Scale };

        id<CAMetalDrawable> Drawable { [Layer nextDrawable] };

        if (Drawable == nullptr)
        {
            return false;
        }

        m_DepthStencil->UpdateTexture(m_Device.Get(),
            { static_cast<i32>(Drawable.texture.width), static_cast<i32>(Drawable.texture.height) });

        CommandQueue* Queue { m_Device->GetCommandQueue() };
        CommandBuffer* Buffer { Queue->BeginBuffer() };
        Buffer->BeginEncoding(ClearColor, Drawable, m_DepthStencil->Texture());
    }

    return true;
}

void Renderer::EndRender(Platform::Window*)
{
    @autoreleasepool
    {
        m_Device->GetCommandQueue()->EndBuffer();
    }
}

void Renderer::SetViewportRect(const ViewportRect&)
{
}

void Renderer::SetScissor(const Recti&)
{
}

u32 Renderer::CreateGraphicsPipeline(const GraphicsPipelineDescription&)
{
    return 0;
}

bool Renderer::BindGraphicsPipeline(u32)
{
    return false;
}

void Renderer::DrawIndexed(u32, u32, u32, u32, u32)
{
}

u32 Renderer::CreateVertexBuffer(const VertexBufferDescription&)
{
    return 0;
}

bool Renderer::UploadVertexData(u32, const VertexDataDescription&)
{
    return false;
}

bool Renderer::BindVertexBuffer(u32)
{
    return false;
}

void Renderer::UpdateViewMatrix(const Matrix4f&)
{
}

Texture* Renderer::GetTexture(u32 ID) const
{
    for (const UniquePtr<Texture>& Texture_ : m_Textures)
    {
        if (Texture_->ID() == ID)
        {
            return Texture_.Get();
        }
    }

    return nullptr;
}

}
}
}

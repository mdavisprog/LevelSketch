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
#include "../../Core/Console.hpp"
#include "../../Core/Math/Rect.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "../../Platform/FileSystem.hpp"
#include "../../Platform/Mac/WindowBridge.hpp"
#include "../../Platform/Window.hpp"
#include "../TextureDescription.hpp"
#include "../ViewportRect.hpp"
#include "CommandBuffer.hpp"
#include "CommandEncoder.hpp"
#include "CommandQueue.hpp"
#include "DepthStencil.hpp"
#include "Device.hpp"
#include "GraphicsPipeline.hpp"
#include "Texture.hpp"
#include "VertexBuffer.hpp"
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
    return Platform::FileSystem::CombinePaths(Platform::FileSystem::ShadersDirectory(), "Metal");
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

u32 Renderer::CreateTexture(const TextureDescription& Description)
{
    UniquePtr<Texture> Texture_ { UniquePtr<Texture>::New() };

    if (!Texture_->Initialize(m_Device.Get(), Description.Width, Description.Height))
    {
        return 0;
    }

    if (!Texture_->Upload(Description.Data, Description.Width * BytesPerPixel(Description.Format)))
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

        const f32 AspectRatio { static_cast<f32>(Layer.drawableSize.width) /
                                static_cast<f32>(Layer.drawableSize.height) };

        const Rectf Bounds { 0.0f,
            0.0f,
            static_cast<f32>(Layer.drawableSize.width),
            static_cast<f32>(Layer.drawableSize.height) };

        m_Uniforms.Perspective = Core::Math::PerspectiveMatrixLH(45.0f, AspectRatio, 0.1f, 100.0f);
        m_Uniforms.Orthographic = Core::Math::OrthographicMatrixLH(Bounds, -1.0f, 1.0f);

        CommandQueue* Queue { m_Device->GetCommandQueue() };
        CommandBuffer* Buffer { Queue->BeginBuffer() };
        CommandEncoder* Encoder { Buffer->BeginEncoding(ClearColor, Drawable, m_DepthStencil->Texture()) };

        if (Encoder == nullptr)
        {
            return false;
        }

        SetViewportRect({ Bounds, 0.0f, 1.0f });

        [Encoder->Get() setFrontFacingWinding:MTLWindingCounterClockwise];
        [Encoder->Get() setVertexBytes:&m_Uniforms length:sizeof(m_Uniforms) atIndex:1];
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

void Renderer::SetViewportRect(const ViewportRect& Rect)
{
    CommandEncoder* Encoder { m_Device->GetCommandQueue()->CurrentBuffer()->CurrentEncoder() };
    if (Encoder == nullptr)
    {
        return;
    }

    @autoreleasepool
    {
        const MTLViewport Viewport { .originX = Rect.Bounds.X,
            .originY = Rect.Bounds.Y,
            .width = Rect.Bounds.W,
            .height = Rect.Bounds.H,
            .znear = Rect.MinDepth,
            .zfar = Rect.MaxDepth };

        [Encoder->Get() setViewport:Viewport];
    }
}

void Renderer::SetScissor(const Recti& Rect)
{
    CommandEncoder* Encoder { m_Device->GetCommandQueue()->CurrentBuffer()->CurrentEncoder() };
    if (Encoder == nullptr)
    {
        return;
    }

    @autoreleasepool
    {
        const MTLScissorRect Scissor { .x = static_cast<NSUInteger>(Rect.X),
            .y = static_cast<NSUInteger>(Rect.Y),
            .width = static_cast<NSUInteger>(Rect.W),
            .height = static_cast<NSUInteger>(Rect.H) };

        [Encoder->Get() setScissorRect:Scissor];
    }
}

u32 Renderer::CreateGraphicsPipeline(const GraphicsPipelineDescription& Description)
{
    UniquePtr<GraphicsPipeline> Pipeline { UniquePtr<GraphicsPipeline>::New() };

    if (!Pipeline->Initialize(m_Device.Get(), Description))
    {
        return 0;
    }

    const u32 Result { Pipeline->ID() };
    m_GraphicsPipelines.Push(std::move(Pipeline));
    return Result;
}

bool Renderer::BindGraphicsPipeline(u32 ID)
{
    GraphicsPipeline* Pipeline { GetGraphicsPipeline(ID) };

    if (Pipeline == nullptr)
    {
        Core::Console::Warning("Failed to find graphics pipeline with ID '%d' for BindGraphicsPipeline.", ID);
        return false;
    }

    CommandEncoder* Encoder { m_Device->GetCommandQueue()->CurrentBuffer()->CurrentEncoder() };
    [Encoder->Get() setRenderPipelineState:Pipeline->Get()];
    [Encoder->Get() setCullMode:Pipeline->CullMode()];
    [Encoder->Get() setDepthStencilState:Pipeline->DepthStencil()];

    return true;
}

void Renderer::DrawIndexed(u32 IndexCount, u32, u32 StartIndex, u32 BaseVertex, u32)
{
    CommandEncoder* Encoder { m_Device->GetCommandQueue()->CurrentBuffer()->CurrentEncoder() };
    if (Encoder == nullptr)
    {
        return;
    }

    VertexBuffer const* Buffer { Encoder->Buffer() };
    if (Buffer == nullptr)
    {
        Core::Console::Warning("No bound vertex buffer for DrawIndexed.");
        return;
    }

    @autoreleasepool
    {
        [Encoder->Get() setVertexBufferOffset:BaseVertex * Buffer->Stride() atIndex:0];
        [Encoder->Get() drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                   indexCount:IndexCount
                                    indexType:Buffer->IndexType()
                                  indexBuffer:Buffer->GetIndexBuffer()
                            indexBufferOffset:StartIndex * Buffer->IndexTypeSize()];
    }
}

u32 Renderer::CreateVertexBuffer(const VertexBufferDescription& Description)
{
    UniquePtr<VertexBuffer> Buffer { UniquePtr<VertexBuffer>::New() };

    if (!Buffer->Initialize(m_Device.Get(), Description))
    {
        return 0;
    }

    const u32 Result { Buffer->ID() };
    m_VertexBuffers.Push(std::move(Buffer));
    return Result;
}

bool Renderer::UploadVertexData(u32 ID, const VertexDataDescription& Description)
{
    VertexBuffer* Buffer { GetVertexBuffer(ID) };

    if (Buffer == nullptr)
    {
        Core::Console::Warning("Failed to find vertex buffer with ID '%d' in UploadVertexData.", ID);
        return false;
    }

    Buffer->Upload(Description);

    return true;
}

bool Renderer::BindVertexBuffer(u32 ID)
{
    VertexBuffer* Buffer { GetVertexBuffer(ID) };

    if (Buffer == nullptr)
    {
        Core::Console::Warning("Failed to find vertex buffer with ID '%d' in BindVertexBuffer.", ID);
        return false;
    }

    m_Device->GetCommandQueue()->CurrentBuffer()->CurrentEncoder()->Bind(Buffer);

    return true;
}

void Renderer::UpdateViewMatrix(const Matrix4f& View)
{
    m_Uniforms.View = View;
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

VertexBuffer* Renderer::GetVertexBuffer(u32 ID) const
{
    for (const UniquePtr<VertexBuffer>& Buffer : m_VertexBuffers)
    {
        if (Buffer->ID() == ID)
        {
            return Buffer.Get();
        }
    }

    return nullptr;
}

GraphicsPipeline* Renderer::GetGraphicsPipeline(u32 ID) const
{
    for (const UniquePtr<GraphicsPipeline>& Pipeline : m_GraphicsPipelines)
    {
        if (Pipeline->ID() == ID)
        {
            return Pipeline.Get();
        }
    }

    return nullptr;
}

}
}
}

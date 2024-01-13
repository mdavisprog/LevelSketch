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

#include "RenderBridge.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Math/Vertex.hpp"
#include "../../External/OctaneGUI/VertexBuffer.h"
#include "../../Platform/FileSystem.hpp"
#include "../../Platform/Window.hpp"
#include "Shader.hpp"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

RenderBridge::RenderBridge()
{
}

bool RenderBridge::Initialize(CAMetalLayer* Layer, Platform::Window* Window)
{
    @autoreleasepool
    {
        if (m_Device != nullptr)
        {
            return true;
        }

        m_Device = Layer.device;
        m_CommandQueue = [m_Device newCommandQueue];

        MTLDepthStencilDescriptor* DepthStencilDesc = [MTLDepthStencilDescriptor new];
        DepthStencilDesc.depthCompareFunction = MTLCompareFunctionLess;
        DepthStencilDesc.depthWriteEnabled = YES;
        m_DepthStencil = [m_Device newDepthStencilStateWithDescriptor:DepthStencilDesc];

        String Path { Platform::FileSystem::ApplicationDirectory() + "/Content/Shaders/Metal/Test.metal" };
        Shader Shader_;
        if (!Shader_.LoadFile(m_Device, Path.Data()))
        {
            return false;
        }

        if (!Shader_.LoadVertex("VertexMain")) { return false; }
        if (!Shader_.LoadPixel("PixelMain")) { return false; }

        MTLRenderPipelineDescriptor* PipelineDesc = [MTLRenderPipelineDescriptor new];
        PipelineDesc.label = @"Test";
        PipelineDesc.vertexFunction = Shader_.Vertex();
        PipelineDesc.fragmentFunction = Shader_.Pixel();
        PipelineDesc.colorAttachments[0].pixelFormat = Layer.pixelFormat;
        PipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

        PipelineDesc.vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
        PipelineDesc.vertexDescriptor.attributes[0].bufferIndex = 0;
        PipelineDesc.vertexDescriptor.attributes[0].offset = 0;
        PipelineDesc.vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;
        PipelineDesc.vertexDescriptor.attributes[1].bufferIndex = 0;
        PipelineDesc.vertexDescriptor.attributes[1].offset = sizeof(Vector3);
        PipelineDesc.vertexDescriptor.attributes[2].format = MTLVertexFormatFloat4;
        PipelineDesc.vertexDescriptor.attributes[2].bufferIndex = 0;
        PipelineDesc.vertexDescriptor.attributes[2].offset = sizeof(Vector3) + sizeof(Vector2);
        PipelineDesc.vertexDescriptor.layouts[0].stepRate = 1;
        PipelineDesc.vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        PipelineDesc.vertexDescriptor.layouts[0].stride = sizeof(Vertex3);

        NSError* Error { nullptr };
        m_PipelineState = [m_Device newRenderPipelineStateWithDescriptor:PipelineDesc error:&Error];
        if (m_PipelineState == nil)
        {
            Core::Console::Error("Failed to create render pipeline state.");
            Core::Console::Error("Error: %s", [[Error localizedDescription] UTF8String]);
            return false;
        }

        //
        // GUI Render Pass
        //

        MTLDepthStencilDescriptor* DepthStencilDescGUI = [MTLDepthStencilDescriptor new];
        DepthStencilDescGUI.depthCompareFunction = MTLCompareFunctionAlways;
        DepthStencilDescGUI.depthWriteEnabled = NO;
        m_DepthStencilGUI = [m_Device newDepthStencilStateWithDescriptor:DepthStencilDescGUI];

        Path = Platform::FileSystem::ApplicationDirectory() + "/Content/Shaders/Metal/GUI.metal";
        Shader_.Clear();
        if (!Shader_.LoadFile(m_Device, Path.Data()))
        {
            return false;
        }

        if (!Shader_.LoadVertex("VertexMain")) { return false; }
        if (!Shader_.LoadPixel("PixelMain")) { return false; }

        PipelineDesc.label = @"GUI";
        PipelineDesc.vertexFunction = Shader_.Vertex();
        PipelineDesc.fragmentFunction = Shader_.Pixel();
        PipelineDesc.rasterSampleCount = 1;
        PipelineDesc.colorAttachments[0].pixelFormat = Layer.pixelFormat;
        PipelineDesc.colorAttachments[0].blendingEnabled = YES;
        PipelineDesc.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        PipelineDesc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        PipelineDesc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        PipelineDesc.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        PipelineDesc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
        PipelineDesc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        PipelineDesc.depthAttachmentPixelFormat = MTLPixelFormatInvalid;

        PipelineDesc.vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
        PipelineDesc.vertexDescriptor.attributes[0].bufferIndex = 0;
        PipelineDesc.vertexDescriptor.attributes[0].offset = 0;
        PipelineDesc.vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;
        PipelineDesc.vertexDescriptor.attributes[1].bufferIndex = 0;
        PipelineDesc.vertexDescriptor.attributes[1].offset = sizeof(OctaneGUI::Vector2);
        PipelineDesc.vertexDescriptor.attributes[2].format = MTLVertexFormatUChar4;
        PipelineDesc.vertexDescriptor.attributes[2].bufferIndex = 0;
        PipelineDesc.vertexDescriptor.attributes[2].offset = sizeof(OctaneGUI::Vector2) + sizeof(OctaneGUI::Vector2);
        PipelineDesc.vertexDescriptor.layouts[0].stepRate = 1;
        PipelineDesc.vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        PipelineDesc.vertexDescriptor.layouts[0].stride = sizeof(OctaneGUI::Vertex);

        m_PipelineStateGUI = [m_Device newRenderPipelineStateWithDescriptor:PipelineDesc error:&Error];
        if (m_PipelineStateGUI == nil)
        {
            Core::Console::Error("Failed to create render pipeline state.");
            Core::Console::Error("Error: %s", [[Error localizedDescription] UTF8String]);
            return false;
        }

        UpdateDepthBuffer(Layer.drawableSize);

        m_RenderBuffer.Initialize(m_Device, 1000, 1000);
        m_RenderBufferGUI.Initialize(m_Device, 100000, 100000);

        const f32 Offset { 0.5f };
        const Vertex3 Vertices[3]
        {
            {{0.0f, Offset, 5.0f}, {0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{-Offset, -Offset, 5.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
            {{Offset, -Offset, 5.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
        };
        m_RenderBuffer.UploadVertexData(Vertices, sizeof(Vertices));

        const u32 Indices[3] { 0, 1, 2 };
        m_RenderBuffer.UploadIndexData(Indices, sizeof(Indices));

        // FIXME: Take into account scale.
        const Rectf Bounds
        {
            0.0f, 0.0f,
            static_cast<f32>(Window->Size().X), static_cast<f32>(Window->Size().Y)
        };

        m_Uniforms.Projection = Core::Math::PerspectiveMatrixRH(75.0f, Window->AspectRatio(), 0.1f, 1000.0f).Transpose();
        m_Uniforms.Orthographic = Core::Math::OrthographicMatrixRH(Bounds, -1.0f, 1.0f).Transpose();

        const u8 WhiteTexture[4] { 255, 255, 255, 255 };
        const u32 ID = LoadTexture(WhiteTexture, 1, 1, 4);
        m_WhiteTexture = GetTexture(ID);
    }

    return true;
}

void RenderBridge::Render(CAMetalLayer* Layer, Platform::Window*)
{
    @autoreleasepool
    {
        if (Layer == nil)
        {
            return;
        }

        id<MTLCommandBuffer> CommandBuffer = [m_CommandQueue commandBuffer];
        id<CAMetalDrawable> Drawable = [Layer nextDrawable];
        if (Drawable == nil)
        {
            return;
        }

        // TODO: Depth buffer should be updated whenever the size of the view changes.
        // Probably can be done through a window delegate.
        if (m_DepthBuffer == nil)
        {
            UpdateDepthBuffer(Layer.drawableSize);
        }

        MTLRenderPassDescriptor* RenderPassDesc = [MTLRenderPassDescriptor new];
        RenderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
        RenderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
        RenderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 1.0, 1.0, 1.0);
        RenderPassDesc.colorAttachments[0].texture = Drawable.texture;

        RenderPassDesc.depthAttachment.loadAction = MTLLoadActionClear;
        RenderPassDesc.depthAttachment.storeAction = MTLStoreActionDontCare;
        RenderPassDesc.depthAttachment.clearDepth = 1.0;
        RenderPassDesc.depthAttachment.texture = m_DepthBuffer;

        // FIXME: Scale size.
        MTLViewport Viewport
        {
            .originX = 0.0,
            .originY = 0.0,
            .width = Layer.drawableSize.width,
            .height = Layer.drawableSize.height,
            .znear = 0.0,
            .zfar = 1.0
        };

        id<MTLRenderCommandEncoder> Encoder = [CommandBuffer renderCommandEncoderWithDescriptor:RenderPassDesc];
        [Encoder setRenderPipelineState:m_PipelineState];
        [Encoder setViewport:Viewport];
        [Encoder setFrontFacingWinding:MTLWindingCounterClockwise];
        [Encoder setCullMode:MTLCullModeBack];
        [Encoder setDepthStencilState:m_DepthStencil];
        [Encoder setVertexBuffer:m_RenderBuffer.Vertex() offset:0 atIndex:0];
        [Encoder setVertexBytes:&m_Uniforms length:sizeof(m_Uniforms) atIndex:1];
        [Encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                            indexCount:3
                             indexType:MTLIndexTypeUInt32
                           indexBuffer:m_RenderBuffer.Index()
                     indexBufferOffset:0];
        [Encoder endEncoding];

        //
        // GUI Render Pass
        //

        MTLRenderPassDescriptor* RenderPassDescGUI = [MTLRenderPassDescriptor new];
        RenderPassDescGUI.colorAttachments[0].loadAction = MTLLoadActionLoad;
        RenderPassDescGUI.colorAttachments[0].storeAction = MTLStoreActionStore;
        RenderPassDescGUI.colorAttachments[0].texture = Drawable.texture;

        id<MTLRenderCommandEncoder> EncoderGUI { [CommandBuffer renderCommandEncoderWithDescriptor:RenderPassDescGUI] };
        [EncoderGUI setRenderPipelineState:m_PipelineStateGUI];
        [EncoderGUI setViewport:Viewport];
        [EncoderGUI setFrontFacingWinding:MTLWindingCounterClockwise];
        [EncoderGUI setCullMode:MTLCullModeNone];
        [EncoderGUI setDepthStencilState:m_DepthStencilGUI];
        [EncoderGUI setVertexBuffer:m_RenderBufferGUI.Vertex() offset:0 atIndex:0];
        [EncoderGUI setVertexBytes:&m_Uniforms length:sizeof(m_Uniforms) atIndex:1];

        for (const OctaneGUI::DrawCommand& Command : m_GUICommands)
        {
            id<MTLTexture> Tex { m_WhiteTexture };
            if (Command.TextureID() != 0)
            {
                Tex = GetTexture(Command.TextureID());
            }

            MTLScissorRect Scissor
            {
                .x = 0,
                .y = 0,
                .width = static_cast<NSUInteger>(Layer.drawableSize.width),
                .height = static_cast<NSUInteger>(Layer.drawableSize.height)
            };

            const OctaneGUI::Rect Clip { Command.Clip() };
            if (!Clip.IsZero())
            {
                // FIXME: Apply scale.
                OctaneGUI::Vector2 Min = Clip.Min;// * Scale;
                OctaneGUI::Vector2 Max = Clip.Max;// * Scale;

                Min.X = std::max<f32>(Min.X, 0.0f);
                Min.Y = std::max<f32>(Min.Y, 0.0f);
                Max.X = std::min<f32>(Max.X, static_cast<f32>(Layer.drawableSize.width));// * Scale.X);
                Max.Y = std::min<f32>(Max.Y, static_cast<f32>(Layer.drawableSize.height));// * Scale.Y);

                const OctaneGUI::Vector2 ClipSize = Max - Min;
                Scissor =
                {
                    .x = (NSUInteger)Min.X,
                    .y = (NSUInteger)Min.Y,
                    .width = (NSUInteger)ClipSize.X,
                    .height = (NSUInteger)ClipSize.Y
                };
            }

            [EncoderGUI setScissorRect:Scissor];
            [EncoderGUI setFragmentTexture:Tex atIndex:0];
            [EncoderGUI setVertexBufferOffset:Command.VertexOffset() * sizeof(OctaneGUI::Vertex) atIndex:0];
            [EncoderGUI drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                   indexCount:Command.IndexCount()
                                    indexType:MTLIndexTypeUInt32
                                  indexBuffer:m_RenderBufferGUI.Index()
                            indexBufferOffset:Command.IndexOffset() * sizeof(u32)];
        }

        [EncoderGUI endEncoding];

        [CommandBuffer presentDrawable:Drawable];
        [CommandBuffer commit];
    }
}

u32 RenderBridge::LoadTexture(const void* Data, u32 Width, u32 Height, u8)
{
    Texture Tex;

    if (!Tex.Create(m_Device, Width, Height))
    {
        return 0;
    }

    if (!Tex.Upload(Data))
    {
        return 0;
    }

    const u32 Result { Tex.ID() };
    m_Textures.Push(std::move(Tex));
    return Result;
}

void RenderBridge::UploadGUIData(const OctaneGUI::VertexBuffer& Buffer)
{
    if (!m_RenderBufferGUI.Initialized())
    {
        return;
    }

    m_RenderBufferGUI.UploadVertexData(
        Buffer.GetVertices().data(),
        static_cast<u64>(Buffer.GetVertexCount() * sizeof(OctaneGUI::Vertex))
    );

    m_RenderBufferGUI.UploadIndexData(
        Buffer.GetIndices().data(),
        static_cast<u64>(Buffer.GetIndexCount() * sizeof(u32))
    );

    m_GUICommands
        .Clear()
        .Reserve(Buffer.Commands().size());
    
    for (const OctaneGUI::DrawCommand& Command : Buffer.Commands())
    {
        m_GUICommands.Push(Command);
    }
}

RenderBridge& RenderBridge::UpdateDepthBuffer(const CGSize& Size)
{
    if (Size.width == 0 || Size.height == 0)
    {
        return *this;
    }

    MTLTextureDescriptor* DepthBufferDesc = [MTLTextureDescriptor new];
    DepthBufferDesc.width = Size.width;
    DepthBufferDesc.height = Size.height;
    DepthBufferDesc.pixelFormat = MTLPixelFormatDepth32Float;
    DepthBufferDesc.storageMode = MTLStorageModePrivate;
    DepthBufferDesc.usage = MTLTextureUsageRenderTarget;

    m_DepthBuffer = [m_Device newTextureWithDescriptor:DepthBufferDesc];

    return *this;
}

id<MTLTexture> RenderBridge::GetTexture(u32 ID) const
{
    for (const Texture& Item : m_Textures)
    {
        if (Item.ID() == ID)
        {
            return Item.Data();
        }
    }

    return nullptr;
}

}
}
}

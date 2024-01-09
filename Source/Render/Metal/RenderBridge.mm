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
#include "../../Platform/FileSystem.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

RenderBridge::RenderBridge()
{
}

bool RenderBridge::Initialize(CAMetalLayer* Layer)
{
    @autoreleasepool
    {
        if (m_Device != nullptr)
        {
            return true;
        }

        m_Device = MTLCreateSystemDefaultDevice();
        m_CommandQueue = [m_Device newCommandQueue];

        m_RenderPassDesc = [MTLRenderPassDescriptor new];
        m_RenderPassDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
        m_RenderPassDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
        m_RenderPassDesc.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 1.0, 1.0, 1.0);

        m_RenderPassDesc.depthAttachment.loadAction = MTLLoadActionClear;
        m_RenderPassDesc.depthAttachment.storeAction = MTLStoreActionDontCare;
        m_RenderPassDesc.depthAttachment.clearDepth = 1.0;

        MTLDepthStencilDescriptor* DepthStencilDesc = [MTLDepthStencilDescriptor new];
        DepthStencilDesc.depthCompareFunction = MTLCompareFunctionLess;
        DepthStencilDesc.depthWriteEnabled = YES;
        m_DepthStencil = [m_Device newDepthStencilStateWithDescriptor:DepthStencilDesc];

        const String Path { Platform::FileSystem::ApplicationDirectory() + "/Content/Shaders/Metal/Test.metal" };
        const String Contents { Platform::FileSystem::ReadContents(Path) };

        NSError* Error = nil;
        id<MTLLibrary> Shaders = [m_Device newLibraryWithSource:[NSString stringWithUTF8String:Contents.Data()]
                                                        options:nil
                                                          error:&Error];
        if (Shaders == nil)
        {
            Core::Console::Error("Failed to load shader source at '%s'.", Path.Data());
            Core::Console::Error("Error: %s", [[Error localizedDescription] UTF8String]);
            return false;
        }

        MTLFunctionConstantValues* ConstantValues = [MTLFunctionConstantValues alloc];
        id<MTLFunction> VertexShader = [Shaders newFunctionWithName:@"VertexMain"
                                                     constantValues:ConstantValues
                                                              error:&Error];
        if (VertexShader == nil)
        {
            Core::Console::Error("Failed to load vertex shader.");
            Core::Console::Error("Error: %s", [[Error localizedDescription] UTF8String]);
            return false;
        }

        id<MTLFunction> PixelShader = [Shaders newFunctionWithName:@"PixelMain"
                                                    constantValues:ConstantValues
                                                             error:&Error];
        if (PixelShader  == nil)
        {
            Core::Console::Error("Failed to load pixel shader.");
            Core::Console::Error("Error: %s", [[Error localizedDescription] UTF8String]);
            return false;
        }

        MTLRenderPipelineDescriptor* PipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
        PipelineDesc.label = @"Test";
        PipelineDesc.vertexFunction = VertexShader;
        PipelineDesc.fragmentFunction = PixelShader;
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
        PipelineDesc.vertexDescriptor.layouts[0].stride = sizeof(Vertex3);

        m_PipelineState = [m_Device newRenderPipelineStateWithDescriptor:PipelineDesc error:&Error];
        if (m_PipelineState == nil)
        {
            Core::Console::Error("Failed to create render pipeline state.");
            Core::Console::Error("Error: %s", [[Error localizedDescription] UTF8String]);
            return false;
        }

        UpdateDepthBuffer(Layer.drawableSize);

        m_RenderBuffer.Initialize(m_Device, 1000, 1000);

        const f32 Offset { 0.5f };
        const Vertex3 Vertices[3]
        {
            {{0.0f, Offset, 0.0f}, {0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{-Offset, -Offset, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
            {{Offset, -Offset, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
        };
        m_RenderBuffer.UploadVertexData(Vertices, sizeof(Vertices));

        const u32 Indices[3] { 0, 1, 2 };
        m_RenderBuffer.UploadIndexData(Indices, sizeof(Indices));
    }

    return true;
}

void RenderBridge::Render(CAMetalLayer* Layer)
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

    m_RenderPassDesc.colorAttachments[0].texture = Drawable.texture;

    MTLViewport Viewport
    {
        .originX = 0.0,
        .originY = 0.0,
        .width = Layer.drawableSize.width,
        .height = Layer.drawableSize.height,
        .znear = 0.0,
        .zfar = 1.0
    };

    id<MTLRenderCommandEncoder> Encoder = [CommandBuffer renderCommandEncoderWithDescriptor:m_RenderPassDesc];
    [Encoder setRenderPipelineState:m_PipelineState];
    [Encoder setViewport:Viewport];
    [Encoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [Encoder setCullMode:MTLCullModeBack];
    [Encoder setDepthStencilState:m_DepthStencil];
    [Encoder setVertexBuffer:m_RenderBuffer.Vertex() offset:0 atIndex:0];
    [Encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                        indexCount:3
                         indexType:MTLIndexTypeUInt32
                       indexBuffer:m_RenderBuffer.Index()
                 indexBufferOffset:0];
    [Encoder endEncoding];

    [CommandBuffer presentDrawable:Drawable];
    [CommandBuffer commit];
}

RenderBridge& RenderBridge::UpdateDepthBuffer(CGSize Size)
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
    m_RenderPassDesc.depthAttachment.texture = m_DepthBuffer;

    return *this;
}

}
}
}

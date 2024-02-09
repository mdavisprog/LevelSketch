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

#include "GraphicsPipeline.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/FileSystem.hpp"
#include "../GraphicsPipelineDescription.hpp"
#include "../Renderer.hpp"
#include "Device.hpp"
#include "Shader.hpp"

#import <Metal/Metal.h>

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

static String ShaderPath(const String& FileName)
{
    return Platform::FileSystem::SetExtension(Renderer::ShaderPath(FileName.Data()), "metal");
}

static MTLVertexFormat ToFormat(VertexFormat Format)
{
    switch (Format)
    {
    case VertexFormat::Byte: return MTLVertexFormatUChar;
    case VertexFormat::Byte2: return MTLVertexFormatUChar2;
    case VertexFormat::Byte4: return MTLVertexFormatUChar4;
    case VertexFormat::Float: return MTLVertexFormatFloat;
    case VertexFormat::Float2: return MTLVertexFormatFloat2;
    case VertexFormat::Float3: return MTLVertexFormatFloat3;
    case VertexFormat::Float4: return MTLVertexFormatFloat4;
    default: break;
    }

    return MTLVertexFormatInvalid;
}

u32 GraphicsPipeline::s_ID { 0 };

GraphicsPipeline::GraphicsPipeline()
{
}

bool GraphicsPipeline::Initialize(Device const* Device_, const GraphicsPipelineDescription& Description)
{
    @autoreleasepool
    {
        const ShaderDescription& VertexShaderDesc { Description.VertexShader };

        Shader VertexShader;
        if (!VertexShader.LoadFile(Device_, ShaderPath(VertexShaderDesc.Path).Data()))
        {
            return false;
        }

        id<MTLFunction> VertexShaderFunction { VertexShader.LoadFunction(VertexShaderDesc.Function.Data()) };
        if (VertexShaderFunction == nullptr)
        {
            return false;
        }

        const ShaderDescription& FragmentShaderDesc { Description.FragmentShader };

        Shader FragmentShader;
        if (!FragmentShader.LoadFile(Device_, ShaderPath(FragmentShaderDesc.Path).Data()))
        {
            return false;
        }

        id<MTLFunction> FragmentShaderFunction { FragmentShader.LoadFunction(FragmentShaderDesc.Function.Data()) };
        if (FragmentShaderFunction == nullptr)
        {
            return false;
        }

        MTLRenderPipelineDescriptor* PipelineDesc { [MTLRenderPipelineDescriptor new] };
        PipelineDesc.label = [NSString stringWithUTF8String:Description.Name.Data()];
        PipelineDesc.vertexFunction = VertexShaderFunction;
        PipelineDesc.fragmentFunction = FragmentShaderFunction;
        PipelineDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        PipelineDesc.depthAttachmentPixelFormat =
            Description.UseDepthStencilBuffer ? MTLPixelFormatDepth32Float : MTLPixelFormatInvalid;

        u64 Offset { 0 };
        for (u64 I = 0; I < VertexShaderDesc.VertexDescriptions.Size(); I++)
        {
            const VertexDescription& VertexDesc { VertexShaderDesc.VertexDescriptions[I] };
            PipelineDesc.vertexDescriptor.attributes[I].format = ToFormat(VertexDesc.Format);
            PipelineDesc.vertexDescriptor.attributes[I].bufferIndex = 0;
            PipelineDesc.vertexDescriptor.attributes[I].offset = Offset;

            Offset += VertexFormatSize(VertexDesc.Format);
        }

        PipelineDesc.vertexDescriptor.layouts[0].stepRate = 1;
        PipelineDesc.vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        // Offset should be all of the format sizes added up by this point.
        PipelineDesc.vertexDescriptor.layouts[0].stride = Offset;

        NSError* Error { nullptr };
        m_State = [Device_->Get() newRenderPipelineStateWithDescriptor:PipelineDesc error:&Error];

        if (m_State == nullptr)
        {
            Core::Console::Error("Failed to create graphics pipeline. Error: %s",
                [[Error localizedDescription] UTF8String]);
            return false;
        }
    }

    m_ID = ++s_ID;

    return true;
}

id<MTLRenderPipelineState> GraphicsPipeline::Get() const
{
    return m_State;
}

u32 GraphicsPipeline::ID() const
{
    return m_ID;
}

}
}
}

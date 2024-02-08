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

#include "CommandBuffer.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Math/Color.hpp"
#include "CommandEncoder.hpp"
#include "CommandQueue.hpp"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

CommandBuffer::CommandBuffer()
{
}

CommandBuffer::~CommandBuffer()
{
}

bool CommandBuffer::Initialize(CommandQueue const* Queue)
{
    m_CommandBuffer = [Queue->Get() commandBuffer];

    if (m_CommandBuffer == nullptr)
    {
        Core::Console::Error("Failed to create command buffer.");
        return false;
    }

    return true;
}

id<MTLCommandBuffer> CommandBuffer::Get() const
{
    return m_CommandBuffer;
}

CommandEncoder* CommandBuffer::BeginEncoding(const Colorf& ClearColor,
    id<CAMetalDrawable> Drawable,
    id<MTLTexture> DepthTexture)
{
    if (m_CommandEncoder == nullptr)
    {
        @autoreleasepool
        {
            const MTLClearColor Color { MTLClearColorMake(static_cast<f64>(ClearColor.R),
                static_cast<f64>(ClearColor.G),
                static_cast<f64>(ClearColor.B),
                static_cast<f64>(ClearColor.A)) };

            MTLRenderPassDescriptor* Descriptor { [MTLRenderPassDescriptor new] };
            Descriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            Descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            Descriptor.colorAttachments[0].clearColor = Color;
            Descriptor.colorAttachments[0].texture = Drawable.texture;
            Descriptor.depthAttachment.loadAction = MTLLoadActionClear;
            Descriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
            Descriptor.depthAttachment.clearDepth = 1.0;
            Descriptor.depthAttachment.texture = DepthTexture;

            id<MTLRenderCommandEncoder> Encoder { [m_CommandBuffer renderCommandEncoderWithDescriptor:Descriptor] };

            if (Encoder == nullptr)
            {
                Core::Console::Warning("Failed to create render command encoder.");
                return nullptr;
            }

            m_Drawable = Drawable;

            m_CommandEncoder = UniquePtr<CommandEncoder>::New();
            m_CommandEncoder->Set(Encoder);
        }
    }

    return m_CommandEncoder.Get();
}

CommandEncoder* CommandBuffer::CurrentEncoder() const
{
    return m_CommandEncoder.Get();
}

void CommandBuffer::EndEncoding()
{
    if (m_CommandEncoder == nullptr)
    {
        return;
    }

    @autoreleasepool
    {
        [m_CommandEncoder->Get() endEncoding];

        [m_CommandBuffer presentDrawable:m_Drawable];
        [m_CommandBuffer commit];

        // TODO: Implement a frames in flight system with a fence that is signaled
        // when GPU work is complete.
        [m_CommandBuffer waitUntilCompleted];

        m_Drawable = nullptr;
        m_CommandEncoder = nullptr;
    }
}

}
}
}

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

#pragma once

#include "../../Core/Math/Forwards.hpp"
#include "../../Core/Types.hpp"

#include <vulkan/vulkan.hpp>

namespace LevelSketch
{
namespace Render
{

struct ViewportRect;

namespace Vulkan
{

class Buffer;
class CommandPool;
class Device;
class GraphicsPipeline;
class SwapChain;
class Sync;
class VertexBuffer;

class CommandBuffer final
{
public:
    CommandBuffer();

    void Initialize(VkCommandBuffer Handle);
    void Shutdown(Device const* Device_, CommandPool const* Pool);

    bool BeginSingle() const;
    bool BeginRecord(SwapChain const* SwapChain_, const Colorf& ClearColor) const;
    bool End() const;
    bool EndRecord() const;
    void Reset() const;
    bool Submit(Device const* Device_, Sync const* Sync_) const;
    bool SubmitWait(Device const* Device_) const;

    const CommandBuffer& BindPipeline(GraphicsPipeline const* Pipeline) const;
    const CommandBuffer& BindBuffer(VertexBuffer const* VertexBuffer_) const;
    const CommandBuffer& BindDescriptorSet(GraphicsPipeline const* Pipeline, VkDescriptorSet Set) const;
    const CommandBuffer& DrawVertices(u32 VertexCount, u32 InstanceCount, u32 FirstVertex, u32 FirstInstance) const;
    const CommandBuffer& DrawVerticesIndexed(u32 IndexCount,
        u32 InstanceCount,
        u32 FirstIndex,
        u32 VertexOffset,
        u32 FirstInstance) const;
    const CommandBuffer& SetViewport(const ViewportRect& Rect) const;
    const CommandBuffer& SetScissor(const Recti& Rect) const;
    const CommandBuffer& CopyBuffer(Buffer const* From, Buffer const* To, u64 Size) const;

private:
    VkCommandBuffer m_CommandBuffer { VK_NULL_HANDLE };
};

}
}
}

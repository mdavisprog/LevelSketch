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

#include "../../Core/Types.hpp"
#include "vulkan/vulkan.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

class CommandPool;
class Device;
class GraphicsPipeline;
class RenderBuffer;
class SwapChain;
class Sync;

class CommandBuffer
{
public:
    CommandBuffer();

    void Initialize(VkCommandBuffer Handle);
    void Shutdown(const Device& Device_, const CommandPool& Pool);

    bool BeginRecord(const GraphicsPipeline& Pipeline, const SwapChain& SwapChain_, u32 FrameIndex) const;
    bool EndRecord() const;
    void Reset() const;
    bool Submit(const Device& Device_, const Sync& Sync_) const;
    const CommandBuffer& BindBuffers(const RenderBuffer& Buffers) const;
    const CommandBuffer& BindDescriptorSet(const GraphicsPipeline& Pipeline, u64 FrameIndex) const;
    const CommandBuffer& DrawVertices(u32 VertexCount, u32 InstanceCount, u32 FirstVertex, u32 FirstInstance) const;
    const CommandBuffer& DrawVerticesIndexed(u32 IndexCount,
        u32 InstanceCount,
        u32 FirstIndex,
        u32 VertexOffset,
        u32 FirstInstance) const;

    bool IsValid() const;

private:
    VkCommandBuffer m_Handle { VK_NULL_HANDLE };
};

}
}
}
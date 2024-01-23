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
#include "CommandPool.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "GraphicsPipeline.hpp"
#include "RenderBuffer.hpp"
#include "SwapChain.hpp"
#include "Sync.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

CommandBuffer::CommandBuffer()
{
}

void CommandBuffer::Initialize(VkCommandBuffer Handle)
{
    m_Handle = Handle;
}

bool CommandBuffer::BeginRecord(const GraphicsPipeline& Pipeline, const SwapChain& SwapChain_, u32 FrameIndex) const
{
    VkCommandBufferBeginInfo BeginInfo {};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = 0;
    BeginInfo.pInheritanceInfo = nullptr;

    VkResult Result { vkBeginCommandBuffer(m_Handle, &BeginInfo) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to begin command buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    const VkClearValue ClearValue {{{ 0.0f, 0.0f, 1.0f, 1.0f }}};

    VkRenderPassBeginInfo RenderPassInfo {};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassInfo.renderPass = Pipeline.RenderPass();
    RenderPassInfo.framebuffer = SwapChain_.Framebuffer(FrameIndex);
    RenderPassInfo.renderArea.offset = { 0, 0 };
    RenderPassInfo.renderArea.extent = SwapChain_.Extents();
    RenderPassInfo.clearValueCount = 1;
    RenderPassInfo.pClearValues = &ClearValue;

    vkCmdBeginRenderPass(m_Handle, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(m_Handle, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline.Handle());

    VkViewport Viewport {};
    Viewport.x = 0;
    Viewport.y = 0;
    Viewport.width = static_cast<f32>(SwapChain_.Extents().width);
    Viewport.height = static_cast<f32>(SwapChain_.Extents().height);
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_Handle, 0, 1, &Viewport);

    VkRect2D Scissor {};
    Scissor.offset = { 0, 0 };
    Scissor.extent = SwapChain_.Extents();
    vkCmdSetScissor(m_Handle, 0, 1, &Scissor);

    return true;
}

bool CommandBuffer::EndRecord() const
{
    vkCmdEndRenderPass(m_Handle);

    VkResult Result = vkEndCommandBuffer(m_Handle);

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to end command buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

void CommandBuffer::Reset() const
{
    if (!IsValid())
    {
        return;
    }

    vkResetCommandBuffer(m_Handle, 0);
}

bool CommandBuffer::Submit(const Device& Device_, const Sync& Sync_) const
{
    if (!IsValid())
    {
        Core::Console::Error("Failed to submit command buffer. Invalid handle.");
        return false;
    }

    const VkSemaphore WaitSemaphores[] = { Sync_.ImageReady() };
    const VkSemaphore SignalSemaphores[] = { Sync_.RenderFinished() };
    const VkPipelineStageFlags WaitFlags[] { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo SubmitInfo {};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = WaitSemaphores;
    SubmitInfo.pWaitDstStageMask = WaitFlags;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &m_Handle;
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = SignalSemaphores;

    VkResult Result { vkQueueSubmit(Device_.GraphicsQueue().Handle(), 1, &SubmitInfo, Sync_.Fence()) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to submit graphics queue. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

const CommandBuffer& CommandBuffer::BindBuffers(const RenderBuffer& Buffers) const
{
    VkBuffer VertexBuffers[] { Buffers.VertexBuffer() };
    VkDeviceSize Offsets[] { 0 };

    vkCmdBindVertexBuffers(m_Handle, 0, 1, VertexBuffers, Offsets);

    return *this;
}

const CommandBuffer& CommandBuffer::DrawVertices(
    u32 VertexCount,
    u32 InstanceCount,
    u32 FirstVertex,
    u32 FirstInstance) const
{
    vkCmdDraw(m_Handle, VertexCount, InstanceCount, FirstVertex, FirstInstance);
    return *this;
}

bool CommandBuffer::IsValid() const
{
    return m_Handle != VK_NULL_HANDLE;
}

}
}
}

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
#include "Buffer.hpp"
#include "CommandPool.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "GraphicsPipeline.hpp"
#include "LogicalDevice.hpp"
#include "Queue.hpp"
#include "SwapChain.hpp"
#include "Sync.hpp"
#include "VertexBuffer.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

CommandBuffer::CommandBuffer()
{
}

void CommandBuffer::Initialize(VkCommandBuffer CommandBuffer_)
{
    m_CommandBuffer = CommandBuffer_;
}

void CommandBuffer::Shutdown(Device const* Device_, CommandPool const* Pool)
{
    if (m_CommandBuffer != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(Device_->GetLogicalDevice()->Get(), Pool->Get(), 1, &m_CommandBuffer);
        m_CommandBuffer = VK_NULL_HANDLE;
    }
}

bool CommandBuffer::BeginRecord(GraphicsPipeline const* Pipeline, SwapChain const* SwapChain_, u32 FrameIndex) const
{
    VkCommandBufferBeginInfo BeginInfo {};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = 0;
    BeginInfo.pInheritanceInfo = nullptr;

    VkResult Result { vkBeginCommandBuffer(m_CommandBuffer, &BeginInfo) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to begin command buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    const VkClearValue ClearValue { { { 0.0f, 0.0f, 1.0f, 1.0f } } };

    VkRenderPassBeginInfo RenderPassInfo {};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassInfo.renderPass = SwapChain_->RenderPass();
    RenderPassInfo.framebuffer = SwapChain_->Framebuffer(FrameIndex);
    RenderPassInfo.renderArea.offset = { 0, 0 };
    RenderPassInfo.renderArea.extent = SwapChain_->Extents();
    RenderPassInfo.clearValueCount = 1;
    RenderPassInfo.pClearValues = &ClearValue;

    vkCmdBeginRenderPass(m_CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->Get());

    VkViewport Viewport {};
    Viewport.x = 0;
    Viewport.y = 0;
    Viewport.width = static_cast<f32>(SwapChain_->Extents().width);
    Viewport.height = static_cast<f32>(SwapChain_->Extents().height);
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_CommandBuffer, 0, 1, &Viewport);

    VkRect2D Scissor {};
    Scissor.offset = { 0, 0 };
    Scissor.extent = SwapChain_->Extents();
    vkCmdSetScissor(m_CommandBuffer, 0, 1, &Scissor);

    return true;
}

bool CommandBuffer::EndRecord() const
{
    vkCmdEndRenderPass(m_CommandBuffer);

    VkResult Result = vkEndCommandBuffer(m_CommandBuffer);

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to end command buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

void CommandBuffer::Reset() const
{
    vkResetCommandBuffer(m_CommandBuffer, 0);
}

bool CommandBuffer::Submit(Device const* Device_, Sync const* Sync_) const
{
    const VkSemaphore WaitSemaphores[] = { Sync_->ImageReady() };
    const VkSemaphore SignalSemaphores[] = { Sync_->RenderFinished() };
    const VkPipelineStageFlags WaitFlags[] { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo SubmitInfo {};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = WaitSemaphores;
    SubmitInfo.pWaitDstStageMask = WaitFlags;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &m_CommandBuffer;
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = SignalSemaphores;

    VkResult Result { vkQueueSubmit(Device_->GraphicsQueue()->Get(), 1, &SubmitInfo, Sync_->Fence()) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to submit graphics queue. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

const CommandBuffer& CommandBuffer::BindBuffer(VertexBuffer const* VertexBuffer_) const
{
    VkBuffer VertexBuffers[] { VertexBuffer_->GetVertexBuffer()->Get() };
    VkDeviceSize Offsets[] { 0 };

    vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, VertexBuffers, Offsets);
    vkCmdBindIndexBuffer(m_CommandBuffer, VertexBuffer_->GetIndexBuffer()->Get(), 0, VertexBuffer_->IndexType());

    return *this;
}

const CommandBuffer& CommandBuffer::BindDescriptorSet(GraphicsPipeline const* Pipeline, VkDescriptorSet Set) const
{
    vkCmdBindDescriptorSets(m_CommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        Pipeline->GetLayout(),
        0,
        1,
        &Set,
        0,
        nullptr);

    return *this;
}

const CommandBuffer& CommandBuffer::DrawVertices(u32 VertexCount,
    u32 InstanceCount,
    u32 FirstVertex,
    u32 FirstInstance) const
{
    vkCmdDraw(m_CommandBuffer, VertexCount, InstanceCount, FirstVertex, FirstInstance);
    return *this;
}

const CommandBuffer& CommandBuffer::DrawVerticesIndexed(u32 IndexCount,
    u32 InstanceCount,
    u32 FirstIndex,
    u32 VertexOffset,
    u32 FirstInstance) const
{
    vkCmdDrawIndexed(m_CommandBuffer, IndexCount, InstanceCount, FirstIndex, VertexOffset, FirstInstance);
    return *this;
}

}
}
}

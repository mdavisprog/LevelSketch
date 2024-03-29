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
#include "../../Core/Math/Rect.hpp"
#include "../ViewportRect.hpp"
#include "Buffer.hpp"
#include "CommandPool.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "GraphicsPipeline.hpp"
#include "LogicalDevice.hpp"
#include "Queue.hpp"
#include "SwapChain.hpp"
#include "Sync.hpp"
#include "Texture.hpp"
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

bool CommandBuffer::BeginSingle() const
{
    VkCommandBufferBeginInfo BeginInfo {};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult Result { vkBeginCommandBuffer(m_CommandBuffer, &BeginInfo) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to begin single command buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

bool CommandBuffer::BeginRecord(SwapChain const* SwapChain_, const Colorf& ClearColor) const
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

    VkClearValue ClearValue {};
    ClearValue.color.float32[0] = ClearColor.R;
    ClearValue.color.float32[1] = ClearColor.G;
    ClearValue.color.float32[2] = ClearColor.B;
    ClearValue.color.float32[3] = ClearColor.A;

    const VkExtent2D Extents { SwapChain_->Extents() };

    VkRenderPassBeginInfo RenderPassInfo {};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassInfo.renderPass = SwapChain_->RenderPass();
    RenderPassInfo.framebuffer = SwapChain_->Framebuffer();
    RenderPassInfo.renderArea.offset = { 0, 0 };
    RenderPassInfo.renderArea.extent = Extents;
    RenderPassInfo.clearValueCount = 1;
    RenderPassInfo.pClearValues = &ClearValue;

    vkCmdBeginRenderPass(m_CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    return true;
}

bool CommandBuffer::End() const
{
    VkResult Result = vkEndCommandBuffer(m_CommandBuffer);

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to end command buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

bool CommandBuffer::EndRecord() const
{
    vkCmdEndRenderPass(m_CommandBuffer);

    if (!End())
    {
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

bool CommandBuffer::SubmitWait(Device const* Device_) const
{
    VkSubmitInfo SubmitInfo {};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &m_CommandBuffer;

    VkResult Result { vkQueueSubmit(Device_->GraphicsQueue()->Get(), 1, &SubmitInfo, VK_NULL_HANDLE) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to submit command buffer to queue. Error: %s", Errors::ToString(Result));
        return false;
    }

    Result = vkQueueWaitIdle(Device_->GraphicsQueue()->Get());

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to wait on command buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

const CommandBuffer& CommandBuffer::BindPipeline(GraphicsPipeline const* Pipeline) const
{
    vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->Get());

    return *this;
}

const CommandBuffer& CommandBuffer::BindBuffer(VertexBuffer const* VertexBuffer_) const
{
    VkBuffer VertexBuffers[] { VertexBuffer_->GetVertexBuffer()->Get() };
    VkDeviceSize Offsets[] { 0 };

    vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, VertexBuffers, Offsets);
    vkCmdBindIndexBuffer(m_CommandBuffer, VertexBuffer_->GetIndexBuffer()->Get(), 0, VertexBuffer_->IndexType());

    return *this;
}

const CommandBuffer& CommandBuffer::BindDescriptorSets(GraphicsPipeline const* Pipeline,
    u32 NumSets,
    const VkDescriptorSet* Sets) const
{
    vkCmdBindDescriptorSets(m_CommandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        Pipeline->GetLayout(),
        0,
        NumSets,
        Sets,
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

const CommandBuffer& CommandBuffer::SetViewport(const ViewportRect& Rect) const
{
    VkViewport Viewport {};
    Viewport.x = Rect.Bounds.X;
    Viewport.y = Rect.Bounds.Y;
    Viewport.width = Rect.Bounds.W;
    Viewport.height = Rect.Bounds.H;
    Viewport.minDepth = Rect.MinDepth;
    Viewport.maxDepth = Rect.MaxDepth;
    vkCmdSetViewport(m_CommandBuffer, 0, 1, &Viewport);

    return *this;
}

const CommandBuffer& CommandBuffer::SetScissor(const Recti& Rect) const
{
    VkRect2D Scissor {};
    Scissor.offset = { Rect.X, Rect.Y };
    Scissor.extent = { static_cast<u32>(Rect.W), static_cast<u32>(Rect.H) };
    vkCmdSetScissor(m_CommandBuffer, 0, 1, &Scissor);

    return *this;
}

const CommandBuffer& CommandBuffer::CopyBuffer(Buffer const* From, Buffer const* To, u64 Size) const
{
    VkBufferCopy Region {};
    Region.srcOffset = 0;
    Region.dstOffset = 0;
    Region.size = Size;

    vkCmdCopyBuffer(m_CommandBuffer, From->Get(), To->Get(), 1, &Region);

    return *this;
}

const CommandBuffer& CommandBuffer::CopyBufferToTexture(Buffer const* From,
    Texture const* To,
    u32 Width,
    u32 Height) const
{
    VkBufferImageCopy Region {};
    Region.bufferOffset = 0;
    Region.bufferRowLength = 0;
    Region.bufferImageHeight = 0;
    Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Region.imageSubresource.mipLevel = 0;
    Region.imageSubresource.baseArrayLayer = 0;
    Region.imageSubresource.layerCount = 1;
    Region.imageOffset = { 0, 0, 0 };
    Region.imageExtent = { Width, Height, 1 };

    vkCmdCopyBufferToImage(m_CommandBuffer, From->Get(), To->Get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);

    return *this;
}

const CommandBuffer& CommandBuffer::TextureBarrier(Texture const* Texture_,
    VkFormat,
    VkImageLayout From,
    VkImageLayout To) const
{
    VkImageMemoryBarrier Barrier {};
    Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    Barrier.oldLayout = From;
    Barrier.newLayout = To;
    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.image = Texture_->Get();
    Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Barrier.subresourceRange.baseMipLevel = 0;
    Barrier.subresourceRange.levelCount = 1;
    Barrier.subresourceRange.baseArrayLayer = 0;
    Barrier.subresourceRange.layerCount = 1;
    Barrier.srcAccessMask = 0;
    Barrier.dstAccessMask = 0;

    VkPipelineStageFlags SourceStage {};
    VkPipelineStageFlags DestStage {};

    if (From == VK_IMAGE_LAYOUT_UNDEFINED && To == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (From == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && To == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DestStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        Core::Console::Error("Unsupported texture barrier '%d' => '%d'", From, To);
        return *this;
    }

    vkCmdPipelineBarrier(m_CommandBuffer, SourceStage, DestStage, 0, 0, nullptr, 0, nullptr, 1, &Barrier);

    return *this;
}

}
}
}

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
#include "SwapChain.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

CommandBuffer::CommandBuffer()
{
}

bool CommandBuffer::Initialize(const Device& Device_, const CommandPool& Pool)
{
    VkCommandBufferAllocateInfo AllocInfo {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = Pool.Handle();
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = 1;

    VkResult Result { vkAllocateCommandBuffers(Device_.GetLogicalDevice().Handle(), &AllocInfo, &m_Handle) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to allocate command buffers. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

bool CommandBuffer::Record(const GraphicsPipeline& Pipeline, const SwapChain& SwapChain_)
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
    RenderPassInfo.framebuffer = SwapChain_.CurrentFramebuffer();
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

    vkCmdDraw(m_Handle, 3, 1, 0, 0);

    vkCmdEndRenderPass(m_Handle);

    Result = vkEndCommandBuffer(m_Handle);

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to end command buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

}
}
}

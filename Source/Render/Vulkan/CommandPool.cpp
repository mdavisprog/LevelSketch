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

#include "CommandPool.hpp"
#include "../../Core/Console.hpp"
#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

CommandPool::CommandPool()
{
}

CommandPool::~CommandPool()
{
}

bool CommandPool::Initialize(Device const* Device_)
{
    VkCommandPoolCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    CreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    CreateInfo.queueFamilyIndex = Device_->GetPhysicalDevice()->GetQueueFamily().Graphics.Value();

    VkResult Result { vkCreateCommandPool(Device_->GetLogicalDevice()->Get(), &CreateInfo, nullptr, &m_CommandPool) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create command pool. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

bool CommandPool::InitializeBuffers(Device const* Device_, u64 Count)
{
    VkCommandBufferAllocateInfo AllocInfo {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = m_CommandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = Count;

    Array<VkCommandBuffer> Buffers {};
    Buffers.Resize(Count);

    VkResult Result { vkAllocateCommandBuffers(Device_->GetLogicalDevice()->Get(), &AllocInfo, Buffers.Data()) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to allocate command buffers. Error: %s", Errors::ToString(Result));
        return false;
    }

    for (u64 I = 0; I < Count; I++)
    {
        UniquePtr<CommandBuffer> Buffer { UniquePtr<CommandBuffer>::New() };
        Buffer->Initialize(Buffers[I]);
        m_CommandBuffers.Push(std::move(Buffer));
    }

    return true;
}

void CommandPool::Shutdown(Device const* Device_)
{
    for (const UniquePtr<CommandBuffer>& Buffer : m_CommandBuffers)
    {
        Buffer->Shutdown(Device_, this);
    }

    m_CommandBuffers.Clear();

    if (m_CommandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(Device_->GetLogicalDevice()->Get(), m_CommandPool, nullptr);
        m_CommandPool = VK_NULL_HANDLE;
    }
}

VkCommandPool CommandPool::Get() const
{
    return m_CommandPool;
}

CommandBuffer const* CommandPool::Buffer(u64 Index) const
{
    return m_CommandBuffers[Index].Get();
}

}
}
}

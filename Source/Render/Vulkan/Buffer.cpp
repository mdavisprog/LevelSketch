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

#include "Buffer.hpp"
#include "../../Core/Console.hpp"
#include "CommandPool.hpp"
#include "Device.hpp"
#include "Errors.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

Buffer::Buffer()
{
}

bool Buffer::Initialize(const Device& Device_, u64 Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags MemProperties)
{
    VkBufferCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    CreateInfo.size = Size;
    CreateInfo.usage = Usage;
    CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult Result { vkCreateBuffer(Device_.GetLogicalDevice().Handle(), &CreateInfo, nullptr, &m_Handle) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create vertex buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    VkMemoryRequirements MemoryReqs {};
    vkGetBufferMemoryRequirements(Device_.GetLogicalDevice().Handle(), m_Handle, &MemoryReqs);

    VkPhysicalDeviceMemoryProperties MemoryProps {};
    vkGetPhysicalDeviceMemoryProperties(Device_.GetPhysicalDevice().Handle(), &MemoryProps);

    u32 MemoryType { UINT32_MAX };
    for (u32 I = 0; I < MemoryProps.memoryTypeCount; I++)
    {
        if ((MemoryReqs.memoryTypeBits & (1 << I)) &&
            (MemoryProps.memoryTypes[I].propertyFlags & MemProperties) == MemProperties)
        {
            MemoryType = I;
            break;
        }
    }

    if (MemoryType == UINT32_MAX)
    {
        Core::Console::Error("Failed to findy memory type.");
        return false;
    }

    VkMemoryAllocateInfo AllocInfo {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = MemoryReqs.size;
    AllocInfo.memoryTypeIndex = MemoryType;

    Result = vkAllocateMemory(Device_.GetLogicalDevice().Handle(), &AllocInfo, nullptr, &m_Memory);

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to allocate memory. Error: %s", Errors::ToString(Result));
        return false;
    }

    Result = vkBindBufferMemory(Device_.GetLogicalDevice().Handle(), m_Handle, m_Memory, 0);

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to bind vertex buffer memory. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

void Buffer::Shutdown(const Device& Device_)
{
    Unmap(Device_);

    if (m_Memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(Device_.GetLogicalDevice().Handle(), m_Memory, nullptr);
        m_Memory = VK_NULL_HANDLE;
    }

    if (m_Handle != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(Device_.GetLogicalDevice().Handle(), m_Handle, nullptr);
        m_Handle = VK_NULL_HANDLE;
    }
}

bool Buffer::Map(const Device& Device_, u64 Size)
{
    if (m_Ptr != nullptr)
    {
        return true;
    }

    VkResult Result { vkMapMemory(Device_.GetLogicalDevice().Handle(), m_Memory, 0, Size, 0, &m_Ptr) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to map vertex memory. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

void Buffer::MapData(const void* Data, u64 Size) const
{
    if (m_Ptr == nullptr)
    {
        return;
    }

    std::memcpy(m_Ptr, Data, Size);
}

void Buffer::Unmap(const Device& Device_)
{
    if (m_Ptr == nullptr)
    {
        return;
    }

    vkUnmapMemory(Device_.GetLogicalDevice().Handle(), m_Memory);
    m_Ptr = nullptr;
}

bool Buffer::Upload(const Device& Device_, const CommandPool& Pool, const void* Data, u64 Size) const
{
    // TODO: Would be nice to create a scoped class to handle automatic
    // shutdown for failure cases. Currently shutting down manually.

    Buffer Staging {};

    if (!Staging.Initialize(Device_,
            Size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        Core::Console::Error("Failed to create staging buffer for upload.");
        return false;
    }

    if (!Staging.Map(Device_, Size))
    {
        Staging.Shutdown(Device_);
        Core::Console::Error("Failed to map staging buffer.");
        return false;
    }

    Staging.MapData(Data, Size);
    Staging.Unmap(Device_);

    VkCommandBufferAllocateInfo AllocInfo {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandPool = Pool.Handle();
    AllocInfo.commandBufferCount = 1;

    VkCommandBuffer CommandBuffer { VK_NULL_HANDLE };

    VkResult Result { vkAllocateCommandBuffers(Device_.GetLogicalDevice().Handle(), &AllocInfo, &CommandBuffer) };

    if (Result != VK_SUCCESS)
    {
        Staging.Shutdown(Device_);
        Core::Console::Error("Failed to allocate staging command buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    auto CleanupFn = [&]() -> void {
        vkFreeCommandBuffers(Device_.GetLogicalDevice().Handle(), Pool.Handle(), 1, &CommandBuffer);
        Staging.Shutdown(Device_);
    };

    VkCommandBufferBeginInfo BeginInfo {};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    Result = vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

    if (Result != VK_SUCCESS)
    {
        CleanupFn();
        Core::Console::Error("Failed to begin staging command buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    VkBufferCopy Region {};
    Region.srcOffset = 0;
    Region.dstOffset = 0;
    Region.size = Size;

    vkCmdCopyBuffer(CommandBuffer, Staging.Handle(), m_Handle, 1, &Region);

    Result = vkEndCommandBuffer(CommandBuffer);

    if (Result != VK_SUCCESS)
    {
        CleanupFn();
        Core::Console::Error("Failed to end staging command buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    VkSubmitInfo SubmitInfo {};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &CommandBuffer;

    Result = vkQueueSubmit(Device_.GraphicsQueue().Handle(), 1, &SubmitInfo, VK_NULL_HANDLE);

    if (Result != VK_SUCCESS)
    {
        CleanupFn();
        Core::Console::Error("Failed to submit copy command to queue. Error: %s", Errors::ToString(Result));
        return false;
    }

    Result = vkQueueWaitIdle(Device_.GraphicsQueue().Handle());

    if (Result != VK_SUCCESS)
    {
        CleanupFn();
        Core::Console::Error("Failed to wait on copy command. Error: %s", Errors::ToString(Result));
        return false;
    }

    CleanupFn();
    return true;
}

VkBuffer Buffer::Handle() const
{
    return m_Handle;
}

bool Buffer::IsValid() const
{
    return m_Handle != VK_NULL_HANDLE;
}

}
}
}

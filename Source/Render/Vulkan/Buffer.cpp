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
#include "../../Core/Memory/UniquePtr.hpp"
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "Queue.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

Buffer::Buffer()
{
}

bool Buffer::Initialize(Device const* Device_, u64 Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags MemProperties)
{
    VkBufferCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    CreateInfo.size = Size;
    CreateInfo.usage = Usage;
    CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult Result { vkCreateBuffer(Device_->GetLogicalDevice()->Get(), &CreateInfo, nullptr, &m_Handle) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create vertex buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    VkMemoryRequirements MemoryReqs {};
    vkGetBufferMemoryRequirements(Device_->GetLogicalDevice()->Get(), m_Handle, &MemoryReqs);

    VkPhysicalDeviceMemoryProperties MemoryProps {};
    vkGetPhysicalDeviceMemoryProperties(Device_->GetPhysicalDevice()->Get(), &MemoryProps);

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

    Result = vkAllocateMemory(Device_->GetLogicalDevice()->Get(), &AllocInfo, nullptr, &m_Memory);

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to allocate memory. Error: %s", Errors::ToString(Result));
        return false;
    }

    Result = vkBindBufferMemory(Device_->GetLogicalDevice()->Get(), m_Handle, m_Memory, 0);

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to bind vertex buffer memory. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

void Buffer::Shutdown(Device const* Device_)
{
    Unmap(Device_);

    if (m_Memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(Device_->GetLogicalDevice()->Get(), m_Memory, nullptr);
        m_Memory = VK_NULL_HANDLE;
    }

    if (m_Handle != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(Device_->GetLogicalDevice()->Get(), m_Handle, nullptr);
        m_Handle = VK_NULL_HANDLE;
    }
}

bool Buffer::Map(Device const* Device_, u64 Size)
{
    if (m_Ptr != nullptr)
    {
        return true;
    }

    VkResult Result { vkMapMemory(Device_->GetLogicalDevice()->Get(), m_Memory, 0, Size, 0, &m_Ptr) };

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

void Buffer::Unmap(Device const* Device_)
{
    if (m_Ptr == nullptr)
    {
        return;
    }

    vkUnmapMemory(Device_->GetLogicalDevice()->Get(), m_Memory);
    m_Ptr = nullptr;
}

bool Buffer::Upload(Device const* Device_, CommandPool const* Pool, const void* Data, u64 Size) const
{
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

    UniquePtr<CommandBuffer> Commands { Pool->AllocateBuffer(Device_) };
    Commands->BeginSingle();
    Commands->CopyBuffer(&Staging, this, Size);
    Commands->End();
    Commands->SubmitWait(Device_);
    Commands->Shutdown(Device_, Pool);

    Staging.Shutdown(Device_);

    return true;
}

VkBuffer Buffer::Get() const
{
    return m_Handle;
}

}
}
}

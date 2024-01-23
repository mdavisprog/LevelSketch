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

#include "RenderBuffer.hpp"
#include "../../Core/Console.hpp"
#include "Errors.hpp"
#include "Device.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

RenderBuffer::RenderBuffer()
{
}

bool RenderBuffer::Initialize(const Device& Device_, u64 VertexSize)
{
    VkBufferCreateInfo VertexInfo {};
    VertexInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    VertexInfo.size = VertexSize;
    VertexInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VertexInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult Result { vkCreateBuffer(
        Device_.GetLogicalDevice().Handle(),
        &VertexInfo,
        nullptr,
        &m_Vertex)
    };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create vertex buffer. Error: %s", Errors::ToString(Result));
        return false;
    }

    VkMemoryRequirements MemoryReqs {};
    vkGetBufferMemoryRequirements(Device_.GetLogicalDevice().Handle(), m_Vertex, &MemoryReqs);

    VkPhysicalDeviceMemoryProperties MemoryProps {};
    vkGetPhysicalDeviceMemoryProperties(Device_.GetPhysicalDevice().Handle(), &MemoryProps);

    u32 MemoryType { UINT32_MAX };
    const VkMemoryPropertyFlags Flags { VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    for (u32 I = 0; I < MemoryProps.memoryTypeCount; I++)
    {
        if ((MemoryReqs.memoryTypeBits & (1 << I)) &&
            (MemoryProps.memoryTypes[I].propertyFlags & Flags) == Flags)
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

    Result = vkAllocateMemory(
        Device_.GetLogicalDevice().Handle(),
        &AllocInfo,
        nullptr,
        &m_VertexMemory);
    
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to allocate memory. Error: %s", Errors::ToString(Result));
        return false;
    }

    Result = vkBindBufferMemory(
        Device_.GetLogicalDevice().Handle(),
        m_Vertex,
        m_VertexMemory,
        0);
    
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to bind vertex buffer memory. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

void RenderBuffer::Shutdown(const Device& Device_)
{
    if (m_VertexMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(Device_.GetLogicalDevice().Handle(), m_VertexMemory, nullptr);
        m_VertexMemory = VK_NULL_HANDLE;
    }

    if (m_Vertex != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(Device_.GetLogicalDevice().Handle(), m_Vertex, nullptr);
        m_Vertex = VK_NULL_HANDLE;
    }
}

bool RenderBuffer::Map(const Device& Device_, const void* Data, u64 Size) const
{
    void* Ptr { nullptr };

    VkResult Result { vkMapMemory(
        Device_.GetLogicalDevice().Handle(),
        m_VertexMemory,
        0,
        Size,
        0,
        &Ptr)
    };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to map vertex memory. Error: %s", Errors::ToString(Result));
        return false;
    }

    std::memcpy(Ptr, Data, Size);

    vkUnmapMemory(Device_.GetLogicalDevice().Handle(), m_VertexMemory);

    return true;
}

VkBuffer RenderBuffer::VertexBuffer() const
{
    return m_Vertex;
}

}
}
}

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

#include <vulkan/vulkan.hpp>

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

class CommandPool;
class Device;

class Buffer final
{
public:
    static u32 FindMemoryType(Device const* Device_, u32 Filter, VkMemoryPropertyFlags Properties);

    Buffer();

    bool Initialize(Device const* Device_, u64 Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags MemProperties);
    void Shutdown(Device const* Device_);

    bool Map(Device const* Device_, u64 Size);
    void MapData(const void* Data, u64 Size) const;
    void Unmap(Device const* Device_);
    bool Upload(Device const* Device_, CommandPool const* Pool, const void* Data, u64 Size) const;

    VkBuffer Get() const;

private:
    VkBuffer m_Handle { VK_NULL_HANDLE };
    VkDeviceMemory m_Memory { VK_NULL_HANDLE };
    void* m_Ptr { nullptr };
};

}
}
}

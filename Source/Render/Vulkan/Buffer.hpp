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

class Buffer
{
public:
    Buffer();

    bool Initialize(
        const Device& Device_,
        u64 Size,
        VkBufferUsageFlags Usage,
        VkMemoryPropertyFlags MemProperties);
    void Shutdown(const Device& Device_);

    bool Map(const Device& Device_, const void* Data, u64 Size) const;
    bool Upload(const Device& Device_, const CommandPool& Pool, const void* Data, u64 Size) const;

    VkBuffer Handle() const;
    bool IsValid() const;

private:
    VkBuffer m_Handle { VK_NULL_HANDLE };
    VkDeviceMemory m_Memory { VK_NULL_HANDLE };
};

}
}
}

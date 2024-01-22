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
#include "Device.hpp"
#include "Errors.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

CommandPool::CommandPool()
{
}

bool CommandPool::Initialize(const Device& Device_)
{
    VkCommandPoolCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    CreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    CreateInfo.queueFamilyIndex = Device_.GetPhysicalDevice().QueueFamilyIndex().Graphics();

    VkResult Result { vkCreateCommandPool(Device_.GetLogicalDevice().Handle(), &CreateInfo, nullptr, &m_Handle) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create command pool. Error: %s", Errors::ToString(Result));
        return false;
    }

    if (!m_CommandBuffer.Initialize(Device_, *this))
    {
        return false;
    }

    return true;
}

void CommandPool::Shutdown(const Device& Device_)
{
    if (m_Handle != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(Device_.GetLogicalDevice().Handle(), m_Handle, nullptr);
        m_Handle = VK_NULL_HANDLE;
    }
}

bool CommandPool::IsValid() const
{
    return m_Handle != VK_NULL_HANDLE;
}

VkCommandPool CommandPool::Handle() const
{
    return m_Handle;
}

}
}
}


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

#include "Sync.hpp"
#include "../../Core/Console.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "LogicalDevice.hpp"
#include "SwapChain.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

Sync::Sync()
{
}

bool Sync::Initialize(Device const* Device_)
{
    m_ImageReady = CreateSemaphore(Device_);
    if (m_ImageReady == VK_NULL_HANDLE)
    {
        Core::Console::Error("Failed to create image ready semaphore.");
        return false;
    }

    m_RenderFinished = CreateSemaphore(Device_);
    if (m_RenderFinished == VK_NULL_HANDLE)
    {
        Core::Console::Error("Failed to create render finished semaphore.");
        return false;
    }

    VkFenceCreateInfo FenceInfo {};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult Result { vkCreateFence(Device_->GetLogicalDevice()->Get(), &FenceInfo, nullptr, &m_Fence) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create fence. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

void Sync::Shutdown(Device const* Device_)
{
    DestroySemaphore(Device_, m_ImageReady);
    DestroySemaphore(Device_, m_RenderFinished);

    if (m_Fence != VK_NULL_HANDLE)
    {
        vkDestroyFence(Device_->GetLogicalDevice()->Get(), m_Fence, nullptr);
        m_Fence = VK_NULL_HANDLE;
    }
}

u32 Sync::FrameIndex(Device const* Device_, SwapChain const* SwapChain_) const
{
    u32 Index { UINT32_MAX };

    VkResult Result { vkAcquireNextImageKHR(Device_->GetLogicalDevice()->Get(),
        SwapChain_->Get(),
        UINT64_MAX,
        m_ImageReady,
        VK_NULL_HANDLE,
        &Index) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to acquire next image. Error: %s", Errors::ToString(Result));
    }

    return Index;
}

void Sync::WaitForFence(Device const* Device_) const
{
    if (m_Fence == VK_NULL_HANDLE)
    {
        return;
    }

    vkWaitForFences(Device_->GetLogicalDevice()->Get(), 1, &m_Fence, VK_TRUE, UINT64_MAX);
    vkResetFences(Device_->GetLogicalDevice()->Get(), 1, &m_Fence);

    return;
}

VkSemaphore Sync::ImageReady() const
{
    return m_ImageReady;
}

VkSemaphore Sync::RenderFinished() const
{
    return m_RenderFinished;
}

VkFence Sync::Fence() const
{
    return m_Fence;
}

VkSemaphore Sync::CreateSemaphore(Device const* Device_)
{
    VkSemaphoreCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore Handle { VK_NULL_HANDLE };
    VkResult Result { vkCreateSemaphore(Device_->GetLogicalDevice()->Get(), &CreateInfo, nullptr, &Handle) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create semaphore. Error: %s", Errors::ToString(Result));
    }

    return Handle;
}

void Sync::DestroySemaphore(Device const* Device_, VkSemaphore& Semaphore)
{
    if (Semaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(Device_->GetLogicalDevice()->Get(), Semaphore, nullptr);
        Semaphore = VK_NULL_HANDLE;
    }
}

}
}
}

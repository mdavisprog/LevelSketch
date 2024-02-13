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

#include "../../Core/Containers/Array.hpp"

#include <vulkan/vulkan.hpp>

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

class Device;
class Surface;
class Sync;

class SwapChain final
{
public:
    SwapChain();

    bool Initialize(Device const* Device_, Surface const* Surface_);
    void Shutdown(Device const* Device_);

    bool NextFrame(Device const* Device_, Sync const* Sync_);
    bool Present(Device const* Device_, Sync const* Sync_) const;

    VkSwapchainKHR Get() const;
    VkRenderPass RenderPass() const;
    VkFormat Format() const;
    VkExtent2D Extents() const;
    VkFramebuffer Framebuffer() const;

private:
    bool InitializeImageViews(Device const* Device_);
    bool InitializeRenderPass(Device const* Device_);
    bool InitializeFramebuffers(Device const* Device_);

    VkSwapchainKHR m_SwapChain { VK_NULL_HANDLE };
    VkFormat m_Format {};
    VkExtent2D m_Extents {};
    Array<VkImage> m_Images {};
    Array<VkImageView> m_ImageViews {};
    Array<VkFramebuffer> m_Framebuffers {};
    VkRenderPass m_RenderPass { VK_NULL_HANDLE };
    u32 m_Index { 0 };
};

}
}
}

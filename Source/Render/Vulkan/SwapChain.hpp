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
#include "vulkan/vulkan.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

class Device;
class GraphicsPipeline;
class PhysicalDevice;
class Surface;
class Sync;

class SwapChain
{
public:
    struct SupportDetails
    {
        VkSurfaceCapabilitiesKHR SurfaceCapabilities {};
        Array<VkSurfaceFormatKHR> Formats {};
        Array<VkPresentModeKHR> PresentModes {};

        bool IsValid() const
        {
            return !Formats.IsEmpty() && !PresentModes.IsEmpty();
        }
    };

    static SupportDetails GatherDetails(const PhysicalDevice& Device, const Surface& Surface_);

    SwapChain();

    bool Initialize(const Device& Device_, const Surface& Surface_, const VkExtent2D& DefaultExtents);
    bool InitializeFramebuffers(const Device& Device_, const GraphicsPipeline& Pipeline);
    void Shutdown(const Device& Device_);

    bool Present(const Device& Device_, const Sync& Sync_, u32 FrameIndex) const;

    VkSwapchainKHR Handle() const;
    bool IsValid() const;

    VkFormat Format() const;
    VkExtent2D Extents() const;
    VkFramebuffer Framebuffer(u32 Index) const;

private:
    static VkSurfaceFormatKHR BestFormat(const Array<VkSurfaceFormatKHR>& Formats);
    static VkPresentModeKHR BestPresentMode(const Array<VkPresentModeKHR>& Modes);
    static VkExtent2D BestExtents(const VkSurfaceCapabilitiesKHR& Capabilities, const VkExtent2D& DefaultExtents);

    VkSwapchainKHR m_SwapChain { VK_NULL_HANDLE };
    Array<VkImage> m_Images {};
    Array<VkImageView> m_ImageViews {};
    Array<VkFramebuffer> m_Framebuffers {};
    VkFormat m_Format {};
    VkExtent2D m_Extents {};
};

}
}
}

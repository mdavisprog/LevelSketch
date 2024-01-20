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

#include "SwapChain.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Math/Math.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "Surface.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

SwapChain::SupportDetails SwapChain::GatherDetails(const PhysicalDevice& Device, const Surface& Surface_)
{
    SupportDetails Details {};

    if (!Device.IsValid())
    {
        Core::Console::Error("Failed to retrieve swap chain support details. Invalid physical device.");
        return Details;
    }

    if (!Surface_.IsValid())
    {
        Core::Console::Error("Failed to retrieve swap chain support details. Invalid surface.");
        return Details;
    }

    VkResult Result { vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        Device.Handle(),
        Surface_.Handle(),
        &Details.SurfaceCapabilities)
    };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve surface capabilities. Error: %s", Errors::ToString(Result));
        return Details;
    }

    // Formats
    {
        u32 Count { 0 };
        Result = vkGetPhysicalDeviceSurfaceFormatsKHR(Device.Handle(), Surface_.Handle(), &Count, nullptr);

        if (Result != VK_SUCCESS)
        {
            Core::Console::Error("Failed to retrieve surface formats count. Error: %s", Errors::ToString(Result));
            return Details;
        }

        Details.Formats.Resize(Count);
        Result = vkGetPhysicalDeviceSurfaceFormatsKHR(Device.Handle(), Surface_.Handle(), &Count, Details.Formats.Data());

        if (Result != VK_SUCCESS)
        {
            Core::Console::Error("Failed to retrieve surface formats. Error: %s", Errors::ToString(Result));
            return Details;
        }
    }

    // Present Modes
    {
        u32 Count { 0 };
        Result = vkGetPhysicalDeviceSurfacePresentModesKHR(Device.Handle(), Surface_.Handle(), &Count, nullptr);

        if (Result != VK_SUCCESS)
        {
            Core::Console::Error("Failed to retrieve surface present modes count. Error: %s", Errors::ToString(Result));
            return Details;
        }

        Details.PresentModes.Resize(Count);
        Result = vkGetPhysicalDeviceSurfacePresentModesKHR(Device.Handle(), Surface_.Handle(), &Count, Details.PresentModes.Data());

        if (Result != VK_SUCCESS)
        {
            Core::Console::Error("Failed to retrieve surface present modes. Error: %s", Errors::ToString(Result));
            return Details;
        }
    }

    return Details;
}

SwapChain::SwapChain()
{
}

bool SwapChain::Initialize(const Device& Device_, const Surface& Surface_, const VkExtent2D& DefaultExtents)
{
    const PhysicalDevice& PhysicalDevice_ { Device_.GetPhysicalDevice() };
    const SupportDetails Details { GatherDetails(PhysicalDevice_, Surface_) };

    if (!Details.IsValid())
    {
        Core::Console::Error("Failed to initialize swap chain. Invalid support details");
        return false;
    }

    const VkSurfaceFormatKHR Format { BestFormat(Details.Formats) };
    const VkPresentModeKHR PresentMode { BestPresentMode(Details.PresentModes) };
    const VkExtent2D Extents { BestExtents(Details.SurfaceCapabilities, DefaultExtents) };
    const u32 ImageCount { Min<u32>(
        Details.SurfaceCapabilities.minImageCount + 1,
        Details.SurfaceCapabilities.maxImageCount > 0 ? Details.SurfaceCapabilities.maxImageCount : 0xFFFFFFFF)
    };

    VkSwapchainCreateInfoKHR CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    CreateInfo.surface = Surface_.Handle();
    CreateInfo.minImageCount = ImageCount;
    CreateInfo.imageFormat = Format.format;
    CreateInfo.imageColorSpace = Format.colorSpace;
    CreateInfo.imageExtent = Extents;
    CreateInfo.imageArrayLayers = 1;
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    CreateInfo.preTransform = Details.SurfaceCapabilities.currentTransform;
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    CreateInfo.presentMode = PresentMode;
    CreateInfo.clipped = VK_TRUE;
    CreateInfo.oldSwapchain = VK_NULL_HANDLE;

    const u32 Indices[] = { PhysicalDevice_.QueueFamilyIndex().Graphics(), PhysicalDevice_.QueueFamilyIndex().Present() };
    if (PhysicalDevice_.QueueFamilyIndex().Graphics() != PhysicalDevice_.QueueFamilyIndex().Present())
    {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        CreateInfo.queueFamilyIndexCount = 2;
        CreateInfo.pQueueFamilyIndices = Indices;
    }
    else
    {
        CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        CreateInfo.queueFamilyIndexCount = 0;
        CreateInfo.pQueueFamilyIndices = nullptr;
    }

    const VkDevice DeviceHandle { Device_.GetLogicalDevice().Handle() };

    VkResult Result { vkCreateSwapchainKHR(DeviceHandle, &CreateInfo, nullptr, &m_SwapChain) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create swap chain. Error: %s", Errors::ToString(Result));
        return false;
    }

    u32 Count { 0 };
    Result = vkGetSwapchainImagesKHR(DeviceHandle, m_SwapChain, &Count, nullptr);

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve swap chain image count. Error: %s", Errors::ToString(Result));
        return false;
    }

    m_Images.Resize(Count);
    
    Result = vkGetSwapchainImagesKHR(DeviceHandle, m_SwapChain, &Count, m_Images.Data());

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve swap chain images. Error: %s", Errors::ToString(Result));
        return false;
    }

    m_Format = Format.format;
    m_Extents = Extents;

    m_ImageViews.Resize(m_Images.Size());

    for (u64 I = 0; I < m_ImageViews.Size(); I++)
    {
        VkImageViewCreateInfo ImageViewInfo {};
        ImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ImageViewInfo.image = m_Images[I];
        ImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ImageViewInfo.format = m_Format;
        ImageViewInfo.components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        };
        ImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ImageViewInfo.subresourceRange.baseMipLevel = 0;
        ImageViewInfo.subresourceRange.levelCount = 1;
        ImageViewInfo.subresourceRange.baseArrayLayer = 0;
        ImageViewInfo.subresourceRange.layerCount = 1;

        Result = vkCreateImageView(DeviceHandle, &ImageViewInfo, nullptr, &m_ImageViews[I]);

        if (Result != VK_SUCCESS)
        {
            Core::Console::Error("Failed to create image view. Error: %s", Errors::ToString(Result));
            return false;
        }
    }

    return true;
}

void SwapChain::Shutdown(const Device& Device_)
{
    for (const VkImageView& ImageView : m_ImageViews)
    {
        vkDestroyImageView(Device_.GetLogicalDevice().Handle(), ImageView, nullptr);
    }

    if (m_SwapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(Device_.GetLogicalDevice().Handle(), m_SwapChain, nullptr);
        m_SwapChain = VK_NULL_HANDLE;
    }
}

bool SwapChain::IsValid() const
{
    return m_SwapChain != VK_NULL_HANDLE;
}

VkSurfaceFormatKHR SwapChain::BestFormat(const Array<VkSurfaceFormatKHR>& Formats)
{
    if (Formats.IsEmpty())
    {
        return {};
    }

    for (const VkSurfaceFormatKHR& Format : Formats)
    {
        if (Format.format == VK_FORMAT_B8G8R8_SRGB && Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return Format;
        }
    }

    return Formats[0];
}

VkPresentModeKHR SwapChain::BestPresentMode(const Array<VkPresentModeKHR>& Modes)
{
    if (!Modes.IsEmpty())
    {
        for (const VkPresentModeKHR& Mode : Modes)
        {
            if (Mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return Mode;
            }
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::BestExtents(const VkSurfaceCapabilitiesKHR& Capabilities, const VkExtent2D& DefaultExtents)
{
    VkExtent2D Result {};

    if (Capabilities.currentExtent.width != 0xFFFFFFFF)
    {
        Result = Capabilities.currentExtent;
    }
    else
    {
        Result.width = Clamp<i32>(DefaultExtents.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
        Result.height = Clamp<i32>(DefaultExtents.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);
    }

    return Result;
}

}
}
}

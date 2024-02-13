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
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "Queue.hpp"
#include "Surface.hpp"
#include "Sync.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

SwapChain::SwapChain()
{
}

static VkSurfaceFormatKHR BestFormat(const Array<VkSurfaceFormatKHR>& Formats)
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

static VkPresentModeKHR BestPresentMode(const Array<VkPresentModeKHR>& Modes)
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

static VkExtent2D BestExtents(const VkSurfaceCapabilitiesKHR& Capabilities, const VkExtent2D& DefaultExtents)
{
    VkExtent2D Result {};

    if (Capabilities.currentExtent.width != 0xFFFFFFFF)
    {
        Result = Capabilities.currentExtent;
    }
    else
    {
        Result.width =
            Clamp<i32>(DefaultExtents.width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
        Result.height =
            Clamp<i32>(DefaultExtents.height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);
    }

    return Result;
}

bool SwapChain::Initialize(Device const* Device_, Surface const* Surface_)
{
    PhysicalDevice const* PhysicalDevice_ { Device_->GetPhysicalDevice() };
    const PhysicalDevice::SupportDetails& SupportDetails { PhysicalDevice_->GetSupportDetails() };
    const VkExtent2D DefaultExtents { static_cast<u32>(Surface_->Resolution().X),
        static_cast<u32>(Surface_->Resolution().Y) };

    const VkSurfaceFormatKHR Format { BestFormat(SupportDetails.Formats) };
    const VkPresentModeKHR PresentMode { BestPresentMode(SupportDetails.PresentModes) };
    const VkExtent2D Extents { BestExtents(SupportDetails.SurfaceCapabilities, DefaultExtents) };
    const u32 ImageCount { Min<u32>(SupportDetails.SurfaceCapabilities.minImageCount + 1,
        SupportDetails.SurfaceCapabilities.maxImageCount > 0 ? SupportDetails.SurfaceCapabilities.maxImageCount
                                                             : 0xFFFFFFFF) };

    VkSwapchainCreateInfoKHR CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    CreateInfo.surface = Surface_->Get();
    CreateInfo.minImageCount = ImageCount;
    CreateInfo.imageFormat = Format.format;
    CreateInfo.imageColorSpace = Format.colorSpace;
    CreateInfo.imageExtent = Extents;
    CreateInfo.imageArrayLayers = 1;
    CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    CreateInfo.preTransform = SupportDetails.SurfaceCapabilities.currentTransform;
    CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    CreateInfo.presentMode = PresentMode;
    CreateInfo.clipped = VK_TRUE;
    CreateInfo.oldSwapchain = VK_NULL_HANDLE;

    const u32 Indices[] = { PhysicalDevice_->GetQueueFamily().Graphics.Value(),
        PhysicalDevice_->GetQueueFamily().Present.Value() };
    if (PhysicalDevice_->GetQueueFamily().Graphics.Value() != PhysicalDevice_->GetQueueFamily().Present.Value())
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

    VkResult Result { vkCreateSwapchainKHR(Device_->GetLogicalDevice()->Get(), &CreateInfo, nullptr, &m_SwapChain) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create swap chain. Error: %s", Errors::ToString(Result));
        return false;
    }

    m_Format = Format.format;
    m_Extents = Extents;

    if (!InitializeImageViews(Device_))
    {
        return false;
    }

    if (!InitializeRenderPass(Device_))
    {
        return false;
    }

    if (!InitializeFramebuffers(Device_))
    {
        return false;
    }

    return true;
}

void SwapChain::Shutdown(Device const* Device_)
{
    const VkDevice DeviceHandle { Device_->GetLogicalDevice()->Get() };

    for (const VkFramebuffer& Framebuffer : m_Framebuffers)
    {
        vkDestroyFramebuffer(DeviceHandle, Framebuffer, nullptr);
    }

    if (m_RenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(DeviceHandle, m_RenderPass, nullptr);
        m_RenderPass = VK_NULL_HANDLE;
    }

    for (const VkImageView& ImageView : m_ImageViews)
    {
        vkDestroyImageView(DeviceHandle, ImageView, nullptr);
    }
    m_ImageViews.Clear();

    if (m_SwapChain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(DeviceHandle, m_SwapChain, nullptr);
        m_SwapChain = VK_NULL_HANDLE;
    }
}

bool SwapChain::NextFrame(Device const* Device_, Sync const* Sync_)
{
    u32 Index { UINT32_MAX };

    VkResult Result { vkAcquireNextImageKHR(Device_->GetLogicalDevice()->Get(),
        m_SwapChain,
        UINT64_MAX,
        Sync_->ImageReady(),
        VK_NULL_HANDLE,
        &Index) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to acquire next image. Error: %s", Errors::ToString(Result));
        return false;
    }

    m_Index = Index;

    return true;
}

bool SwapChain::Present(Device const* Device_, Sync const* Sync_) const
{
    const VkSemaphore SignalSemaphores[] { Sync_->RenderFinished() };
    const VkSwapchainKHR SwapChains[] { m_SwapChain };

    VkPresentInfoKHR PresentInfo {};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = SignalSemaphores;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = SwapChains;
    PresentInfo.pImageIndices = &m_Index;
    PresentInfo.pResults = nullptr;

    VkResult Result { vkQueuePresentKHR(Device_->PresentQueue()->Get(), &PresentInfo) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to present. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

VkSwapchainKHR SwapChain::Get() const
{
    return m_SwapChain;
}

VkRenderPass SwapChain::RenderPass() const
{
    return m_RenderPass;
}

VkFormat SwapChain::Format() const
{
    return m_Format;
}

VkExtent2D SwapChain::Extents() const
{
    return m_Extents;
}

VkFramebuffer SwapChain::Framebuffer() const
{
    return m_Framebuffers[m_Index];
}

bool SwapChain::InitializeImageViews(Device const* Device_)
{
    u32 Count { 0 };
    VkResult Result { vkGetSwapchainImagesKHR(Device_->GetLogicalDevice()->Get(), m_SwapChain, &Count, nullptr) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve swap chain image count. Error: %s", Errors::ToString(Result));
        return false;
    }

    m_Images.Resize(Count);

    Result = vkGetSwapchainImagesKHR(Device_->GetLogicalDevice()->Get(), m_SwapChain, &Count, m_Images.Data());

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve swap chain images. Error: %s", Errors::ToString(Result));
        return false;
    }

    m_ImageViews.Resize(m_Images.Size());

    for (u64 I = 0; I < m_ImageViews.Size(); I++)
    {
        VkImageViewCreateInfo ImageViewInfo {};
        ImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ImageViewInfo.image = m_Images[I];
        ImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ImageViewInfo.format = m_Format;
        ImageViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY };
        ImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ImageViewInfo.subresourceRange.baseMipLevel = 0;
        ImageViewInfo.subresourceRange.levelCount = 1;
        ImageViewInfo.subresourceRange.baseArrayLayer = 0;
        ImageViewInfo.subresourceRange.layerCount = 1;

        Result = vkCreateImageView(Device_->GetLogicalDevice()->Get(), &ImageViewInfo, nullptr, &m_ImageViews[I]);

        if (Result != VK_SUCCESS)
        {
            Core::Console::Error("Failed to create image view. Error: %s", Errors::ToString(Result));
            return false;
        }
    }

    return true;
}

bool SwapChain::InitializeRenderPass(Device const* Device_)
{
    VkAttachmentDescription ColorAttachment {};
    ColorAttachment.format = m_Format;
    ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ColorRef {};
    ColorRef.attachment = 0;
    ColorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription SubpassDesc {};
    SubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubpassDesc.colorAttachmentCount = 1;
    SubpassDesc.pColorAttachments = &ColorRef;

    VkSubpassDependency SubpassDependency {};
    SubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    SubpassDependency.dstSubpass = 0;
    SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependency.srcAccessMask = 0;
    SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    CreateInfo.attachmentCount = 1;
    CreateInfo.pAttachments = &ColorAttachment;
    CreateInfo.subpassCount = 1;
    CreateInfo.pSubpasses = &SubpassDesc;
    CreateInfo.dependencyCount = 1;
    CreateInfo.pDependencies = &SubpassDependency;

    VkResult Result { vkCreateRenderPass(Device_->GetLogicalDevice()->Get(), &CreateInfo, nullptr, &m_RenderPass) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create render pass. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

bool SwapChain::InitializeFramebuffers(Device const* Device_)
{
    m_Framebuffers.Resize(m_ImageViews.Size());

    for (u64 I = 0; I < m_ImageViews.Size(); I++)
    {
        const VkImageView Attachments[] = { m_ImageViews[I] };

        VkFramebufferCreateInfo CreateInfo {};
        CreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        CreateInfo.renderPass = m_RenderPass;
        CreateInfo.attachmentCount = 1;
        CreateInfo.pAttachments = Attachments;
        CreateInfo.width = m_Extents.width;
        CreateInfo.height = m_Extents.height;
        CreateInfo.layers = 1;

        VkResult Result {
            vkCreateFramebuffer(Device_->GetLogicalDevice()->Get(), &CreateInfo, nullptr, &m_Framebuffers[I])
        };

        if (Result != VK_SUCCESS)
        {
            Core::Console::Error("Failed to create framebuffer. Error: %s", Errors::ToString(Result));
            return false;
        }
    }

    return true;
}

}
}
}

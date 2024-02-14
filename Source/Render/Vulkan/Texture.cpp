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

#include "Texture.hpp"
#include "../../Core/Console.hpp"
#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "LogicalDevice.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

VkImageView Texture::CreateView(Device const* Device_, VkImage Image, VkFormat Format)
{
    VkImageViewCreateInfo ImageViewInfo {};
    ImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ImageViewInfo.image = Image;
    ImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ImageViewInfo.format = Format;
    ImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageViewInfo.subresourceRange.baseMipLevel = 0;
    ImageViewInfo.subresourceRange.levelCount = 1;
    ImageViewInfo.subresourceRange.baseArrayLayer = 0;
    ImageViewInfo.subresourceRange.layerCount = 1;

    VkImageView ViewResult { VK_NULL_HANDLE };
    VkResult Result = vkCreateImageView(Device_->GetLogicalDevice()->Get(), &ImageViewInfo, nullptr, &ViewResult);

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create image view. Error: %s", Errors::ToString(Result));
        return VK_NULL_HANDLE;
    }

    return ViewResult;
}

u32 Texture::s_ID { 0 };

Texture::Texture()
{
}

bool Texture::Initialize(Device const* Device_,
    CommandPool const* Pool,
    const void* Data,
    u32 Width,
    u32 Height,
    u8 BytesPerPixel)
{
    VkImageCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    CreateInfo.imageType = VK_IMAGE_TYPE_2D;
    CreateInfo.extent.width = Width;
    CreateInfo.extent.height = Height;
    CreateInfo.extent.depth = 1;
    CreateInfo.mipLevels = 1;
    CreateInfo.arrayLayers = 1;
    CreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    CreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    CreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    CreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    CreateInfo.flags = 0;

    VkResult Result { vkCreateImage(Device_->GetLogicalDevice()->Get(), &CreateInfo, nullptr, &m_Image) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create image. Error: %s", Errors::ToString(Result));
        return false;
    }

    VkMemoryRequirements Requirements {};
    vkGetImageMemoryRequirements(Device_->GetLogicalDevice()->Get(), m_Image, &Requirements);

    VkMemoryAllocateInfo AllocateInfo {};
    AllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocateInfo.allocationSize = Requirements.size;
    AllocateInfo.memoryTypeIndex =
        Buffer::FindMemoryType(Device_, Requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (AllocateInfo.memoryTypeIndex == UINT32_MAX)
    {
        Shutdown(Device_);
        Core::Console::Error("Failed to findy memory type when allocating image memory.");
        return false;
    }

    Result = vkAllocateMemory(Device_->GetLogicalDevice()->Get(), &AllocateInfo, nullptr, &m_Memory);

    if (Result != VK_SUCCESS)
    {
        Shutdown(Device_);
        Core::Console::Error("Failed to allocate image memory. Error: %s", Errors::ToString(Result));
        return false;
    }

    Result = vkBindImageMemory(Device_->GetLogicalDevice()->Get(), m_Image, m_Memory, 0);

    if (Result != VK_SUCCESS)
    {
        Shutdown(Device_);
        Core::Console::Error("Failed to bind image memory. Error: %s", Errors::ToString(Result));
        return false;
    }

    const u64 Size { Width * Height * BytesPerPixel };

    Buffer Staging {};
    if (!Staging.Initialize(Device_,
            Size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        Shutdown(Device_);
        return false;
    }

    Staging.Map(Device_, Size);
    Staging.MapData(Data, Size);
    Staging.Unmap(Device_);

    Transition(Device_, Pool, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    {
        UniquePtr<CommandBuffer> Commands { Pool->AllocateBuffer(Device_) };
        Commands->BeginSingle();
        Commands->CopyBufferToTexture(&Staging, this, Width, Height);
        Commands->End();
        Commands->SubmitWait(Device_);
        Commands->Shutdown(Device_, Pool);
    }
    Transition(Device_, Pool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    Staging.Shutdown(Device_);

    m_ImageView = CreateView(Device_, m_Image, VK_FORMAT_R8G8B8A8_SRGB);

    m_ID = ++s_ID;

    return true;
}

void Texture::Shutdown(Device const* Device_)
{
    if (m_Memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(Device_->GetLogicalDevice()->Get(), m_Memory, nullptr);
        m_Memory = VK_NULL_HANDLE;
    }

    if (m_ImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(Device_->GetLogicalDevice()->Get(), m_ImageView, nullptr);
        m_ImageView = VK_NULL_HANDLE;
    }

    if (m_Image != VK_NULL_HANDLE)
    {
        vkDestroyImage(Device_->GetLogicalDevice()->Get(), m_Image, nullptr);
        m_Image = VK_NULL_HANDLE;
    }
}

VkImage Texture::Get() const
{
    return m_Image;
}

VkImageView Texture::View() const
{
    return m_ImageView;
}

u32 Texture::ID() const
{
    return m_ID;
}

void Texture::Transition(Device const* Device_, CommandPool const* Pool, VkImageLayout From, VkImageLayout To) const
{
    UniquePtr<CommandBuffer> Commands { Pool->AllocateBuffer(Device_) };
    Commands->BeginSingle();
    Commands->TextureBarrier(this, VK_FORMAT_R8G8B8A8_SRGB, From, To);
    Commands->End();
    Commands->SubmitWait(Device_);
    Commands->Shutdown(Device_, Pool);
}

}
}
}

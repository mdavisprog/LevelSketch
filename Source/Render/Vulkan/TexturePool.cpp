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

#include "TexturePool.hpp"
#include "../../Core/Console.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "LogicalDevice.hpp"
#include "Sampler.hpp"
#include "Texture.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

TexturePool::TexturePool()
{
}

TexturePool::~TexturePool()
{
}

bool TexturePool::Initialize(Device const* Device_, u32 Count)
{
    VkDescriptorPoolSize PoolSize {};
    PoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    PoolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo PoolCreateInfo {};
    PoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolCreateInfo.poolSizeCount = 1;
    PoolCreateInfo.pPoolSizes = &PoolSize;
    PoolCreateInfo.maxSets = Count;

    VkResult Result { vkCreateDescriptorPool(Device_->GetLogicalDevice()->Get(), &PoolCreateInfo, nullptr, &m_Pool) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create texture pool. Error: %s", Errors::ToString(Result));
        return false;
    }

    VkDescriptorSetLayoutBinding LayoutBinding {};
    LayoutBinding.binding = 0;
    LayoutBinding.descriptorCount = 1;
    LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    LayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo LayoutCreateInfo {};
    LayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutCreateInfo.bindingCount = 1;
    LayoutCreateInfo.pBindings = &LayoutBinding;

    Result = vkCreateDescriptorSetLayout(Device_->GetLogicalDevice()->Get(), &LayoutCreateInfo, nullptr, &m_Layout);

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create texture pool layout. Error: %s", Errors::ToString(Result));
        return false;
    }

    m_Sampler = UniquePtr<Sampler>::New();
    if (!m_Sampler->Initialize(Device_))
    {
        return false;
    }

    return true;
}

void TexturePool::Shutdown(Device const* Device_)
{
    for (const UniquePtr<Texture>& Texture_ : m_Textures)
    {
        Texture_->Shutdown(Device_);
    }
    m_Textures.Clear();

    m_Sampler->Shutdown(Device_);

    if (m_Pool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(Device_->GetLogicalDevice()->Get(), m_Pool, nullptr);
        m_Pool = VK_NULL_HANDLE;
    }
}

Texture const* TexturePool::AllocateTexture(Device const* Device_,
    CommandPool const* CommandPool_,
    const TextureDescription& Description)
{
    UniquePtr<Texture> Texture_ { UniquePtr<Texture>::New() };

    if (!Texture_->Initialize(Device_, CommandPool_, Description))
    {
        Texture_->Shutdown(Device_);
        return nullptr;
    }

    VkDescriptorSetAllocateInfo AllocInfo {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocInfo.descriptorPool = m_Pool;
    AllocInfo.descriptorSetCount = 1;
    AllocInfo.pSetLayouts = &m_Layout;

    VkDescriptorSet DescriptorSet { VK_NULL_HANDLE };

    VkResult Result { vkAllocateDescriptorSets(Device_->GetLogicalDevice()->Get(), &AllocInfo, &DescriptorSet) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to allocate texture descriptor set. Error: %s", Errors::ToString(Result));
        Texture_->Shutdown(Device_);
        return nullptr;
    }

    VkDescriptorImageInfo ImageInfo {};
    ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ImageInfo.imageView = Texture_->View();
    ImageInfo.sampler = m_Sampler->Get();

    VkWriteDescriptorSet WriteDescriptorSet {};
    WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    WriteDescriptorSet.dstSet = DescriptorSet;
    WriteDescriptorSet.dstBinding = 0;
    WriteDescriptorSet.dstArrayElement = 0;
    WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    WriteDescriptorSet.descriptorCount = 1;
    WriteDescriptorSet.pBufferInfo = nullptr;
    WriteDescriptorSet.pImageInfo = &ImageInfo;
    WriteDescriptorSet.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(Device_->GetLogicalDevice()->Get(), 1, &WriteDescriptorSet, 0, nullptr);
    m_Sets[Texture_->ID()] = DescriptorSet;

    m_Textures.Push(std::move(Texture_));
    return m_Textures.Back().Get();
}

VkDescriptorSetLayout TexturePool::Layout() const
{
    return m_Layout;
}

VkDescriptorSet TexturePool::Set(u32 ID)
{
    return m_Sets[ID];
}

}
}
}

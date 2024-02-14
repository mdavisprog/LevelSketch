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

#include "DescriptorPool.hpp"
#include "../../Core/Console.hpp"
#include "Buffer.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "LogicalDevice.hpp"
#include "Sampler.hpp"
#include "Texture.hpp"
#include "UniformBuffer.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

DescriptorPool::DescriptorPool()
{
}

DescriptorPool::~DescriptorPool()
{
}

bool DescriptorPool::Initialize(Device const* Device_, u32 Count)
{
    // TODO: Specify layout binding in the GraphicsPipelineDescription struct.
    // Force a single uniform buffer binding and single sampler binding for now.
    VkDescriptorSetLayoutBinding UniformBinding {};
    UniformBinding.binding = 0;
    UniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    UniformBinding.descriptorCount = 1;
    UniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    UniformBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding SamplerBinding {};
    SamplerBinding.binding = 1;
    SamplerBinding.descriptorCount = 1;
    SamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    SamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    SamplerBinding.pImmutableSamplers = nullptr;

    Array<VkDescriptorSetLayoutBinding> LayoutBindings { UniformBinding, SamplerBinding };

    if (!CreateDescriptorSetLayout(Device_, LayoutBindings))
    {
        return false;
    }

    VkDescriptorPoolSize PoolSize[2] {};
    PoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    PoolSize[0].descriptorCount = Count;
    PoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    PoolSize[1].descriptorCount = Count;

    VkDescriptorPoolCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    CreateInfo.poolSizeCount = ARRAY_COUNT(PoolSize);
    CreateInfo.pPoolSizes = PoolSize;
    CreateInfo.maxSets = Count;

    VkResult Result {
        vkCreateDescriptorPool(Device_->GetLogicalDevice()->Get(), &CreateInfo, nullptr, &m_DescriptorPool)
    };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create descriptor pool. Error: %s", Errors::ToString(Result));
        return false;
    }

    if (!CreateDescriptorSets(Device_, Count))
    {
        return false;
    }

    m_Sampler = UniquePtr<Sampler>::New();

    if (!m_Sampler->Initialize(Device_))
    {
        return false;
    }

    return true;
}

void DescriptorPool::Shutdown(Device const* Device_)
{
    m_Sampler->Shutdown(Device_);

    if (m_DescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(Device_->GetLogicalDevice()->Get(), m_DescriptorSetLayout, nullptr);
        m_DescriptorSetLayout = VK_NULL_HANDLE;
    }

    if (m_DescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(Device_->GetLogicalDevice()->Get(), m_DescriptorPool, nullptr);
        m_DescriptorPool = VK_NULL_HANDLE;
    }
}

void DescriptorPool::UpdateUniform(Device const* Device_, UniformBuffer const* Buffer, u64 Index)
{
    VkDescriptorBufferInfo BufferInfo {};
    BufferInfo.buffer = Buffer->Get()->Get();
    BufferInfo.offset = 0;
    BufferInfo.range = Buffer->Size();

    VkWriteDescriptorSet WriteDescriptorSet {};
    WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    WriteDescriptorSet.dstSet = m_DescriptorSets[Index];
    WriteDescriptorSet.dstBinding = 0;
    WriteDescriptorSet.dstArrayElement = 0;
    WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    WriteDescriptorSet.descriptorCount = 1;
    WriteDescriptorSet.pBufferInfo = &BufferInfo;
    WriteDescriptorSet.pImageInfo = nullptr;
    WriteDescriptorSet.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(Device_->GetLogicalDevice()->Get(), 1, &WriteDescriptorSet, 0, nullptr);
}

void DescriptorPool::UpdateSampler(Device const* Device_, Texture const* Texture_, u64 Index)
{
    VkDescriptorImageInfo ImageInfo {};
    ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    ImageInfo.imageView = Texture_->View();
    ImageInfo.sampler = m_Sampler->Get();

    VkWriteDescriptorSet WriteDescriptorSet {};
    WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    WriteDescriptorSet.dstSet = m_DescriptorSets[Index];
    WriteDescriptorSet.dstBinding = 1;
    WriteDescriptorSet.dstArrayElement = 0;
    WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    WriteDescriptorSet.descriptorCount = 1;
    WriteDescriptorSet.pBufferInfo = nullptr;
    WriteDescriptorSet.pImageInfo = &ImageInfo;
    WriteDescriptorSet.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(Device_->GetLogicalDevice()->Get(), 1, &WriteDescriptorSet, 0, nullptr);
}

VkDescriptorPool DescriptorPool::Get() const
{
    return m_DescriptorPool;
}

VkDescriptorSetLayout DescriptorPool::GetLayout() const
{
    return m_DescriptorSetLayout;
}

VkDescriptorSet DescriptorPool::GetSet(u64 Index) const
{
    return m_DescriptorSets[Index];
}

bool DescriptorPool::CreateDescriptorSetLayout(Device const* Device_,
    const Array<VkDescriptorSetLayoutBinding>& Bindings)
{
    VkDescriptorSetLayoutCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    CreateInfo.bindingCount = static_cast<u32>(Bindings.Size());
    CreateInfo.pBindings = Bindings.Data();

    VkResult Result {
        vkCreateDescriptorSetLayout(Device_->GetLogicalDevice()->Get(), &CreateInfo, nullptr, &m_DescriptorSetLayout)
    };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create descriptor set layout. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

bool DescriptorPool::CreateDescriptorSets(Device const* Device_, u32 Count)
{
    Array<VkDescriptorSetLayout> Layouts;

    for (u32 I = 0; I < Count; I++)
    {
        Layouts.Push(m_DescriptorSetLayout);
    }

    VkDescriptorSetAllocateInfo AllocInfo {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocInfo.descriptorPool = m_DescriptorPool;
    AllocInfo.descriptorSetCount = Count;
    AllocInfo.pSetLayouts = Layouts.Data();

    m_DescriptorSets.Resize(Layouts.Size());

    VkResult Result {
        vkAllocateDescriptorSets(Device_->GetLogicalDevice()->Get(), &AllocInfo, m_DescriptorSets.Data())
    };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to allocate descriptor sets. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

}
}
}

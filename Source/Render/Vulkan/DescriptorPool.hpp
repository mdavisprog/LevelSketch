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
#include "../../Core/Memory/UniquePtr.hpp"

#include <vulkan/vulkan.hpp>

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

class Device;
class Sampler;
class UniformBuffer;
class Texture;

// TODO: Support batched writing of descriptor sets.

class DescriptorPool final
{
public:
    DescriptorPool();
    ~DescriptorPool();

    bool Initialize(Device const* Device_, u32 Count);
    void Shutdown(Device const* Device_);

    void UpdateUniform(Device const* Device_, UniformBuffer const* Buffer, u64 Index);
    void UpdateSampler(Device const* Device_, Texture const* Texture_, u64 Index);

    VkDescriptorPool Get() const;
    VkDescriptorSetLayout GetLayout() const;
    VkDescriptorSet GetSet(u64 Index) const;

private:
    bool CreateDescriptorSetLayout(Device const* Device_, const Array<VkDescriptorSetLayoutBinding>& Bindings);
    bool CreateDescriptorSets(Device const* Device_, u32 Count);

    VkDescriptorSetLayout m_DescriptorSetLayout { VK_NULL_HANDLE };
    VkDescriptorPool m_DescriptorPool { VK_NULL_HANDLE };
    Array<VkDescriptorSet> m_DescriptorSets {};
    UniquePtr<Sampler> m_Sampler { nullptr };
};

}
}
}

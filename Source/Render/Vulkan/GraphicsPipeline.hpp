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
class Shader;
class SwapChain;
class UniformBuffer;

class GraphicsPipeline
{
public:
    GraphicsPipeline();

    bool Initialize(const Device& Device_, const SwapChain& SwapChain_, const Shader& Vertex, const Shader& Fragment);
    void Shutdown(const Device& Device_);

    GraphicsPipeline& PushLayoutBinding(const VkDescriptorSetLayoutBinding& Binding);
    const GraphicsPipeline& BindUniformBuffer(const Device& Device_, const UniformBuffer* Buffers) const;

    VkRenderPass RenderPass() const;
    VkPipeline Handle() const;
    VkPipelineLayout PipelineLayout() const;
    VkDescriptorSet DescriptorSet(u64 Index) const;

private:
    bool CreatePipelineLayout(const Device& Device_);
    bool CreateRenderPass(const Device& Device_, const SwapChain& SwapChain_);
    bool CreateDescriptorSetLayout(const Device& Device_);
    bool CreateDescriptorPool(const Device& Device_);
    bool CreateDescriptorSets(const Device& Device_);

    VkPipelineLayout m_PipelineLayout { VK_NULL_HANDLE };
    VkRenderPass m_RenderPass { VK_NULL_HANDLE };
    VkPipeline m_Handle { VK_NULL_HANDLE };

    VkDescriptorSetLayout m_DescriptorSetLayout { VK_NULL_HANDLE };
    Array<VkDescriptorSetLayoutBinding> m_LayoutBindings {};

    VkDescriptorPool m_DescriptorPool { VK_NULL_HANDLE };
    Array<VkDescriptorSet> m_DescriptorSets {};
};

}
}
}
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

#include "../../Core/Containers/Forwards.hpp"
#include "../../Core/Types.hpp"

#include <vulkan/vulkan.hpp>

namespace LevelSketch
{
namespace Render
{

struct GraphicsPipelineDescription;

namespace Vulkan
{

class DescriptorPool;
class Device;
class SwapChain;
class UniformBuffer;

class GraphicsPipeline final
{
private:
    static u32 s_ID;

public:
    GraphicsPipeline();

    bool Initialize(Device const* Device_,
        DescriptorPool const* Pool,
        SwapChain const* SwapChain_,
        const GraphicsPipelineDescription& Description);
    void Shutdown(Device const* Device_);

    void BindUniformBuffer(Device const* Device_, UniformBuffer const* UniformBuffer_, VkDescriptorSet Set) const;

    VkPipeline Get() const;
    VkPipelineLayout GetLayout() const;
    u32 ID() const;

private:
    bool CreatePipelineLayout(Device const* Device_, DescriptorPool const* Pool);

    VkPipelineLayout m_PipelineLayout { VK_NULL_HANDLE };
    VkPipeline m_Pipeline { VK_NULL_HANDLE };
    u32 m_ID { 0 };
};

}
}
}

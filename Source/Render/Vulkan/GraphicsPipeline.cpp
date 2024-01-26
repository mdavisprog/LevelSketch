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

#include "GraphicsPipeline.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Containers/Array.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "Renderer.hpp"
#include "Shader.hpp"
#include "SwapChain.hpp"
#include "UniformBuffer.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

GraphicsPipeline::GraphicsPipeline()
{
}

bool GraphicsPipeline::Initialize(const Device& Device_,
    const SwapChain& SwapChain_,
    const Shader& Vertex,
    const Shader& Fragment)
{
    if (!CreatePipelineLayout(Device_))
    {
        return false;
    }

    if (!CreateRenderPass(Device_, SwapChain_))
    {
        return false;
    }

    VkPipelineShaderStageCreateInfo VertexInfo {};
    VertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    VertexInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    VertexInfo.module = Vertex.Handle();
    VertexInfo.pName = "main";

    VkPipelineShaderStageCreateInfo FragmentInfo {};
    FragmentInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    FragmentInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    FragmentInfo.module = Fragment.Handle();
    FragmentInfo.pName = "main";

    VkPipelineShaderStageCreateInfo Stages[] { VertexInfo, FragmentInfo };

    const VkDynamicState DynamicStates[] { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineVertexInputStateCreateInfo VertexInputInfo {};
    VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputInfo.vertexBindingDescriptionCount = static_cast<u32>(Vertex.Bindings().Size());
    VertexInputInfo.pVertexBindingDescriptions = Vertex.Bindings().Data();
    VertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(Vertex.Attributes().Size());
    VertexInputInfo.pVertexAttributeDescriptions = Vertex.Attributes().Data();

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo {};
    InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineDynamicStateCreateInfo DynamicInfo {};
    DynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicInfo.dynamicStateCount = ARRAY_COUNT(DynamicStates);
    DynamicInfo.pDynamicStates = DynamicStates;

    VkPipelineViewportStateCreateInfo ViewportInfo {};
    ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportInfo.viewportCount = 1;
    ViewportInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo RasterInfo {};
    RasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterInfo.depthClampEnable = VK_FALSE;
    RasterInfo.rasterizerDiscardEnable = VK_FALSE;
    RasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
    RasterInfo.lineWidth = 1.0f;
    RasterInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    RasterInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    RasterInfo.depthBiasEnable = VK_FALSE;
    RasterInfo.depthBiasConstantFactor = 0.0f;
    RasterInfo.depthBiasClamp = 0.0f;
    RasterInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo MultisampleInfo {};
    MultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultisampleInfo.sampleShadingEnable = VK_FALSE;
    MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    MultisampleInfo.minSampleShading = 1.0f;
    MultisampleInfo.pSampleMask = nullptr;
    MultisampleInfo.alphaToCoverageEnable = VK_FALSE;
    MultisampleInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState ColorAttachment {};
    ColorAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColorAttachment.blendEnable = VK_FALSE;
    ColorAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    ColorAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    ColorAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    ColorAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo BlendInfo {};
    BlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    BlendInfo.logicOpEnable = VK_FALSE;
    BlendInfo.logicOp = VK_LOGIC_OP_COPY;
    BlendInfo.attachmentCount = 1;
    BlendInfo.pAttachments = &ColorAttachment;
    BlendInfo.blendConstants[0] = 0.0f;
    BlendInfo.blendConstants[1] = 0.0f;
    BlendInfo.blendConstants[2] = 0.0f;
    BlendInfo.blendConstants[3] = 0.0f;

    VkGraphicsPipelineCreateInfo PipelineInfo {};
    PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineInfo.stageCount = 2;
    PipelineInfo.pStages = Stages;
    PipelineInfo.pVertexInputState = &VertexInputInfo;
    PipelineInfo.pInputAssemblyState = &InputAssemblyInfo;
    PipelineInfo.pViewportState = &ViewportInfo;
    PipelineInfo.pRasterizationState = &RasterInfo;
    PipelineInfo.pMultisampleState = &MultisampleInfo;
    PipelineInfo.pDepthStencilState = nullptr;
    PipelineInfo.pColorBlendState = &BlendInfo;
    PipelineInfo.pDynamicState = &DynamicInfo;
    PipelineInfo.layout = m_PipelineLayout;
    PipelineInfo.renderPass = m_RenderPass;
    PipelineInfo.subpass = 0;
    PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    PipelineInfo.basePipelineIndex = -1;

    VkResult Result { vkCreateGraphicsPipelines(Device_.GetLogicalDevice().Handle(),
        VK_NULL_HANDLE,
        1,
        &PipelineInfo,
        nullptr,
        &m_Handle) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create graphics pipeline. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

void GraphicsPipeline::Shutdown(const Device& Device_)
{
    if (m_Handle != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(Device_.GetLogicalDevice().Handle(), m_Handle, nullptr);
        m_Handle = VK_NULL_HANDLE;
    }

    if (m_RenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(Device_.GetLogicalDevice().Handle(), m_RenderPass, nullptr);
        m_RenderPass = VK_NULL_HANDLE;
    }

    if (m_PipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(Device_.GetLogicalDevice().Handle(), m_PipelineLayout, nullptr);
        m_PipelineLayout = VK_NULL_HANDLE;
    }

    if (m_DescriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(Device_.GetLogicalDevice().Handle(), m_DescriptorSetLayout, nullptr);
        m_DescriptorSetLayout = VK_NULL_HANDLE;
    }

    if (m_DescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(Device_.GetLogicalDevice().Handle(), m_DescriptorPool, nullptr);
        m_DescriptorPool = VK_NULL_HANDLE;
    }
}

GraphicsPipeline& GraphicsPipeline::PushLayoutBinding(const VkDescriptorSetLayoutBinding& Binding)
{
    m_LayoutBindings.Push(Binding);
    return *this;
}

const GraphicsPipeline& GraphicsPipeline::BindUniformBuffer(const Device& Device_, const UniformBuffer* Buffers) const
{
    for (u32 I = 0; I < FRAMES_IN_FLIGHT; I++)
    {
        VkDescriptorBufferInfo BufferInfo {};
        BufferInfo.buffer = Buffers[I].Handle();
        BufferInfo.offset = 0;
        BufferInfo.range = sizeof(UniformBuffer::Uniforms);

        VkWriteDescriptorSet WriteDescriptorSet {};
        WriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        WriteDescriptorSet.dstSet = m_DescriptorSets[I];
        WriteDescriptorSet.dstBinding = 0;
        WriteDescriptorSet.dstArrayElement = 0;
        WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        WriteDescriptorSet.descriptorCount = 1;
        WriteDescriptorSet.pBufferInfo = &BufferInfo;
        WriteDescriptorSet.pImageInfo = nullptr;
        WriteDescriptorSet.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(Device_.GetLogicalDevice().Handle(), 1, &WriteDescriptorSet, 0, nullptr);
    }

    return *this;
}

VkRenderPass GraphicsPipeline::RenderPass() const
{
    return m_RenderPass;
}

VkPipeline GraphicsPipeline::Handle() const
{
    return m_Handle;
}

VkPipelineLayout GraphicsPipeline::PipelineLayout() const
{
    return m_PipelineLayout;
}

VkDescriptorSet GraphicsPipeline::DescriptorSet(u64 Index) const
{
    return m_DescriptorSets[Index];
}

bool GraphicsPipeline::CreatePipelineLayout(const Device& Device_)
{
    if (!CreateDescriptorSetLayout(Device_))
    {
        return false;
    }

    if (!CreateDescriptorPool(Device_))
    {
        return false;
    }

    if (!CreateDescriptorSets(Device_))
    {
        return false;
    }

    VkPipelineLayoutCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    CreateInfo.setLayoutCount = 1;
    CreateInfo.pSetLayouts = &m_DescriptorSetLayout;
    CreateInfo.pushConstantRangeCount = 0;
    CreateInfo.pPushConstantRanges = nullptr;

    VkResult Result {
        vkCreatePipelineLayout(Device_.GetLogicalDevice().Handle(), &CreateInfo, nullptr, &m_PipelineLayout)
    };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create pipeline layout. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

bool GraphicsPipeline::CreateRenderPass(const Device& Device_, const SwapChain& SwapChain_)
{
    VkAttachmentDescription ColorAttachment {};
    ColorAttachment.format = SwapChain_.Format();
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

    VkResult Result { vkCreateRenderPass(Device_.GetLogicalDevice().Handle(), &CreateInfo, nullptr, &m_RenderPass) };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create render pass. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

bool GraphicsPipeline::CreateDescriptorSetLayout(const Device& Device_)
{
    VkDescriptorSetLayoutCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    CreateInfo.bindingCount = static_cast<u32>(m_LayoutBindings.Size());
    CreateInfo.pBindings = m_LayoutBindings.Data();

    VkResult Result {
        vkCreateDescriptorSetLayout(Device_.GetLogicalDevice().Handle(), &CreateInfo, nullptr, &m_DescriptorSetLayout)
    };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create descriptor set layout. Error: %s", Errors::ToString(Result));
        return false;
    }

    m_LayoutBindings.Clear();
    return true;
}

bool GraphicsPipeline::CreateDescriptorPool(const Device& Device_)
{
    VkDescriptorPoolSize PoolSize {};
    PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    PoolSize.descriptorCount = static_cast<u32>(FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    CreateInfo.poolSizeCount = 1;
    CreateInfo.pPoolSizes = &PoolSize;
    CreateInfo.maxSets = static_cast<u32>(FRAMES_IN_FLIGHT);

    VkResult Result {
        vkCreateDescriptorPool(Device_.GetLogicalDevice().Handle(), &CreateInfo, nullptr, &m_DescriptorPool)
    };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create descriptor pool. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

bool GraphicsPipeline::CreateDescriptorSets(const Device& Device_)
{
    VkDescriptorSetLayout Layouts[] = { m_DescriptorSetLayout, m_DescriptorSetLayout };
    VkDescriptorSetAllocateInfo AllocInfo {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocInfo.descriptorPool = m_DescriptorPool;
    AllocInfo.descriptorSetCount = static_cast<u32>(FRAMES_IN_FLIGHT);
    AllocInfo.pSetLayouts = Layouts;

    m_DescriptorSets.Resize(FRAMES_IN_FLIGHT);

    VkResult Result {
        vkAllocateDescriptorSets(Device_.GetLogicalDevice().Handle(), &AllocInfo, m_DescriptorSets.Data())
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

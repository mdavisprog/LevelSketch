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
#include "../../Platform/FileSystem.hpp"
#include "../GraphicsPipelineDescription.hpp"
#include "Buffer.hpp"
#include "DescriptorPool.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "LogicalDevice.hpp"
#include "Renderer.hpp"
#include "Shader.hpp"
#include "SwapChain.hpp"
#include "TexturePool.hpp"
#include "UniformBuffer.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

VkFormat ToFormat(VertexFormat Format)
{
    switch (Format)
    {
    case VertexFormat::Byte: return VK_FORMAT_R8_UNORM;
    case VertexFormat::Byte2: return VK_FORMAT_R8G8_UNORM;
    case VertexFormat::Byte4: return VK_FORMAT_R8G8B8A8_UNORM;
    case VertexFormat::Float: return VK_FORMAT_R32_SFLOAT;
    case VertexFormat::Float2: return VK_FORMAT_R32G32_SFLOAT;
    case VertexFormat::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
    case VertexFormat::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
    default: break;
    }

    return VK_FORMAT_UNDEFINED;
}

VkCullModeFlagBits ToCullMode(CullModeType Type)
{
    switch (Type)
    {
    case CullModeType::Back: return VK_CULL_MODE_BACK_BIT;
    case CullModeType::Front: return VK_CULL_MODE_FRONT_BIT;
    case CullModeType::None:
    default: break;
    }

    return VK_CULL_MODE_NONE;
}

GraphicsPipeline::GraphicsPipeline()
{
}

bool GraphicsPipeline::Initialize(Device const* Device_,
    DescriptorPool const* Pool,
    TexturePool const* TexturePool_,
    SwapChain const* SwapChain_,
    const GraphicsPipelineDescription& Description)
{
    if (!CreatePipelineLayout(Device_, Pool, TexturePool_))
    {
        return false;
    }

    const ShaderDescription& VertexShaderDesc { Description.VertexShader };
    Shader VertexShader {};
    if (!VertexShader.Load(Device_,
            Platform::FileSystem::SetExtension(Renderer::ShaderPath(VertexShaderDesc.Path.Data()), "vert").Data()))
    {
        return false;
    }

    Array<VkVertexInputAttributeDescription> Attributes;
    u64 Offset { 0 };
    for (u32 I = 0; I < VertexShaderDesc.VertexDescriptions.Size(); I++)
    {
        const VertexDescription& VertexDesc { VertexShaderDesc.VertexDescriptions[I] };

        VkVertexInputAttributeDescription Attribute {};
        Attribute.binding = 0;
        Attribute.location = I;
        Attribute.format = ToFormat(VertexDesc.Format);
        Attribute.offset = Offset;
        Attributes.Push(Attribute);

        Offset += VertexFormatSize(VertexDesc.Format);
    }

    VkVertexInputBindingDescription VertexBinding {};
    VertexBinding.binding = 0;
    VertexBinding.stride = Offset;
    VertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    VkVertexInputBindingDescription VertexBindings[] { VertexBinding };

    const ShaderDescription& FragmentShaderDesc { Description.FragmentShader };
    Shader FragmentShader {};
    if (!FragmentShader.Load(Device_,
            Platform::FileSystem::SetExtension(Renderer::ShaderPath(FragmentShaderDesc.Path.Data()), "frag").Data()))
    {
        return false;
    }

    const auto CleanupShaders = [&]() -> void
    {
        VertexShader.Shutdown(Device_);
        FragmentShader.Shutdown(Device_);
    };

    VkPipelineShaderStageCreateInfo VertexInfo {};
    VertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    VertexInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    VertexInfo.module = VertexShader.Get();
    VertexInfo.pName = "main";

    VkPipelineShaderStageCreateInfo FragmentInfo {};
    FragmentInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    FragmentInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    FragmentInfo.module = FragmentShader.Get();
    FragmentInfo.pName = "main";

    VkPipelineShaderStageCreateInfo Stages[] { VertexInfo, FragmentInfo };
    const VkDynamicState DynamicStates[] { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineVertexInputStateCreateInfo VertexInputInfo {};
    VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputInfo.vertexBindingDescriptionCount = 1;
    VertexInputInfo.pVertexBindingDescriptions = VertexBindings;
    VertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(Attributes.Size());
    VertexInputInfo.pVertexAttributeDescriptions = Attributes.Data();

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
    RasterInfo.cullMode = ToCullMode(Description.CullMode);
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
    if (Description.UseAlphaBlending)
    {
        ColorAttachment.blendEnable = VK_TRUE;
        ColorAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        ColorAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        ColorAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        ColorAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        ColorAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        ColorAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }
    else
    {
        ColorAttachment.blendEnable = VK_FALSE;
        ColorAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        ColorAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        ColorAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        ColorAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        ColorAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        ColorAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }

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
    PipelineInfo.renderPass = SwapChain_->RenderPass();
    PipelineInfo.subpass = 0;
    PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    PipelineInfo.basePipelineIndex = -1;

    VkResult Result { vkCreateGraphicsPipelines(Device_->GetLogicalDevice()->Get(),
        VK_NULL_HANDLE,
        1,
        &PipelineInfo,
        nullptr,
        &m_Pipeline) };

    if (Result != VK_SUCCESS)
    {
        CleanupShaders();
        Core::Console::Error("Failed to create graphics pipeline. Error: %s", Errors::ToString(Result));
        return false;
    }

    m_Handle = GraphicsPipelineHandle::Acquire();

    CleanupShaders();
    return true;
}

void GraphicsPipeline::Shutdown(Device const* Device_)
{
    if (m_Pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(Device_->GetLogicalDevice()->Get(), m_Pipeline, nullptr);
        m_Pipeline = VK_NULL_HANDLE;
    }

    if (m_PipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(Device_->GetLogicalDevice()->Get(), m_PipelineLayout, nullptr);
        m_PipelineLayout = VK_NULL_HANDLE;
    }
}

VkPipeline GraphicsPipeline::Get() const
{
    return m_Pipeline;
}

VkPipelineLayout GraphicsPipeline::GetLayout() const
{
    return m_PipelineLayout;
}

GraphicsPipelineHandle GraphicsPipeline::Handle() const
{
    return m_Handle;
}

bool GraphicsPipeline::CreatePipelineLayout(Device const* Device_,
    DescriptorPool const* Pool,
    TexturePool const* TexturePool_)
{
    const VkDescriptorSetLayout Layouts[] { Pool->GetLayout(), TexturePool_->Layout() };
    VkPipelineLayoutCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    CreateInfo.setLayoutCount = 2;
    CreateInfo.pSetLayouts = Layouts;
    CreateInfo.pushConstantRangeCount = 0;
    CreateInfo.pPushConstantRanges = nullptr;

    VkResult Result {
        vkCreatePipelineLayout(Device_->GetLogicalDevice()->Get(), &CreateInfo, nullptr, &m_PipelineLayout)
    };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create pipeline layout. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

}
}
}

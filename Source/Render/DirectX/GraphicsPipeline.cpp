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
#include "../../Platform/FileSystem.hpp"
#include "../../Platform/Windows/Errors.hpp"
#include "../GraphicsPipelineDescription.hpp"
#include "Device.hpp"
#include "RootSignature.hpp"
#include "Shader.hpp"

#include <d3dcompiler.h>

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

u32 GraphicsPipeline::s_ID { 0 };

GraphicsPipeline::GraphicsPipeline()
{
}

static DXGI_FORMAT ToFormat(VertexFormat Format)
{
    switch (Format)
    {
    case VertexFormat::Byte: return DXGI_FORMAT_R8_UNORM;
    case VertexFormat::Byte2: return DXGI_FORMAT_R8G8_UNORM;
    case VertexFormat::Byte4: return DXGI_FORMAT_R8G8B8A8_UNORM;
    case VertexFormat::Float: return DXGI_FORMAT_R32_FLOAT;
    case VertexFormat::Float2: return DXGI_FORMAT_R32G32_FLOAT;
    case VertexFormat::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
    case VertexFormat::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    default: break;
    }

    return DXGI_FORMAT_UNKNOWN;
}

static D3D12_CULL_MODE ToCullMode(CullMode Mode)
{
    switch (Mode)
    {
    case CullMode::Back: return D3D12_CULL_MODE_BACK;
    case CullMode::Front: return D3D12_CULL_MODE_FRONT;
    default: break;
    }

    return D3D12_CULL_MODE_NONE;
}

bool GraphicsPipeline::Initialize(Device const* Device_, const GraphicsPipelineDescription& Description)
{
    u32 CompileFlags { 0 };

#if defined(DEBUG)
    CompileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    const ShaderDescription& VertexShaderDesc { Description.VertexShader };
    Shader VertexShader;

    const String VertexShaderPath {
        Platform::FileSystem::SetExtension(Renderer::ShaderPath(VertexShaderDesc.Path.Data()), "hlsl")
    };
    if (!VertexShader.LoadSource(VertexShaderPath.Data()))
    {
        Core::Console::Error("Failed to load vertex shader '%s'.", VertexShaderPath.Data());
        return false;
    }

    VertexShader.SetName(VertexShaderDesc.Name.Data())
        .SetEntryPoint(VertexShaderDesc.Function.Data())
        .SetCompileFlags(CompileFlags)
        .SetTarget("vs_5_0");

    for (const VertexDescription& VertexDesc : VertexShaderDesc.VertexDescriptions)
    {
        D3D12_INPUT_ELEMENT_DESC Element {};
        Element.SemanticName = VertexDesc.Name.Data();
        Element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        Element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        Element.Format = ToFormat(VertexDesc.Format);
        VertexShader.AddInputElement(Element);
    }

    if (!VertexShader.Compile())
    {
        Core::Console::WriteLine("Failed to compile vertex shader '%s'.\nError: %s",
            VertexShaderDesc.Name.Data(),
            static_cast<LPCSTR>(VertexShader.Errors()->GetBufferPointer()));
        return false;
    }

    const ShaderDescription& FragmentShaderDesc { Description.FragmentShader };
    Shader FragmentShader;

    const String FragmentShaderPath {
        Platform::FileSystem::SetExtension(Renderer::ShaderPath(FragmentShaderDesc.Path.Data()), "hlsl")
    };
    if (!FragmentShader.LoadSource(FragmentShaderPath.Data()))
    {
        Core::Console::Error("Failed to load fragment shader '%s'.", FragmentShaderPath.Data());
        return false;
    }

    FragmentShader.SetName(FragmentShaderDesc.Name.Data())
        .SetEntryPoint(FragmentShaderDesc.Function.Data())
        .SetCompileFlags(CompileFlags)
        .SetTarget("ps_5_0");

    if (!FragmentShader.Compile())
    {
        Core::Console::WriteLine("Failed to compile fragment shader '%s'.\nError: %s",
            FragmentShaderDesc.Name.Data(),
            static_cast<LPCSTR>(FragmentShader.Errors()->GetBufferPointer()));
        return false;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsDesc {};
    GraphicsDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    GraphicsDesc.RasterizerState.CullMode = ToCullMode(Description.CullMode);
    GraphicsDesc.RasterizerState.FrontCounterClockwise = TRUE;
    GraphicsDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    GraphicsDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    GraphicsDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    GraphicsDesc.RasterizerState.DepthClipEnable = TRUE;
    GraphicsDesc.RasterizerState.MultisampleEnable = FALSE;
    GraphicsDesc.RasterizerState.AntialiasedLineEnable = FALSE;
    GraphicsDesc.RasterizerState.ForcedSampleCount = 0;
    GraphicsDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    GraphicsDesc.BlendState.AlphaToCoverageEnable = FALSE;
    GraphicsDesc.BlendState.IndependentBlendEnable = FALSE;
    for (UINT I = 0; I < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++I)
    {
        if (Description.UseAlphaBlending)
        {
            GraphicsDesc.BlendState.RenderTarget[I].BlendEnable = TRUE;
            GraphicsDesc.BlendState.RenderTarget[I].LogicOpEnable = FALSE;
            GraphicsDesc.BlendState.RenderTarget[I].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            GraphicsDesc.BlendState.RenderTarget[I].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
            GraphicsDesc.BlendState.RenderTarget[I].BlendOp = D3D12_BLEND_OP_ADD;
            GraphicsDesc.BlendState.RenderTarget[I].SrcBlendAlpha = D3D12_BLEND_ONE;
            GraphicsDesc.BlendState.RenderTarget[I].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
            GraphicsDesc.BlendState.RenderTarget[I].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            GraphicsDesc.BlendState.RenderTarget[I].LogicOp = D3D12_LOGIC_OP_NOOP;
            GraphicsDesc.BlendState.RenderTarget[I].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        }
        else
        {
            GraphicsDesc.BlendState.RenderTarget[I].BlendEnable = FALSE;
            GraphicsDesc.BlendState.RenderTarget[I].LogicOpEnable = FALSE;
            GraphicsDesc.BlendState.RenderTarget[I].SrcBlend = D3D12_BLEND_ONE;
            GraphicsDesc.BlendState.RenderTarget[I].DestBlend = D3D12_BLEND_ZERO;
            GraphicsDesc.BlendState.RenderTarget[I].BlendOp = D3D12_BLEND_OP_ADD;
            GraphicsDesc.BlendState.RenderTarget[I].SrcBlendAlpha = D3D12_BLEND_ONE;
            GraphicsDesc.BlendState.RenderTarget[I].DestBlendAlpha = D3D12_BLEND_ZERO;
            GraphicsDesc.BlendState.RenderTarget[I].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            GraphicsDesc.BlendState.RenderTarget[I].LogicOp = D3D12_LOGIC_OP_NOOP;
            GraphicsDesc.BlendState.RenderTarget[I].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        }
    }

    GraphicsDesc.SampleMask = UINT_MAX;
    GraphicsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    GraphicsDesc.NumRenderTargets = 1;
    GraphicsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    GraphicsDesc.SampleDesc.Count = 1;

    GraphicsDesc.InputLayout = { VertexShader.InputElements(), VertexShader.NumInputElements() };
    GraphicsDesc.pRootSignature = Device_->GetRootSignature()->Get();
    GraphicsDesc.VS = { VertexShader.Blob()->GetBufferPointer(), VertexShader.Blob()->GetBufferSize() };
    GraphicsDesc.PS = { FragmentShader.Blob()->GetBufferPointer(), FragmentShader.Blob()->GetBufferSize() };

    GraphicsDesc.DepthStencilState.DepthEnable = Description.UseDepthStencilBuffer;
    GraphicsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    GraphicsDesc.DepthStencilState.DepthFunc =
        Description.UseDepthStencilBuffer ? D3D12_COMPARISON_FUNC_LESS_EQUAL : D3D12_COMPARISON_FUNC_NEVER;
    GraphicsDesc.DepthStencilState.StencilEnable = Description.UseDepthStencilBuffer;
    GraphicsDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    GraphicsDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    GraphicsDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    GraphicsDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    GraphicsDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    GraphicsDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    GraphicsDesc.DepthStencilState.BackFace = GraphicsDesc.DepthStencilState.FrontFace;
    GraphicsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    HRESULT Result { Device_->Get()->CreateGraphicsPipelineState(&GraphicsDesc, IID_PPV_ARGS(&m_State)) };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to create graphics pipeline state '%s'. Error: %s",
            Description.Name.Data(),
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    m_ID = ++s_ID;
    return true;
}

u32 GraphicsPipeline::ID() const
{
    return m_ID;
}

ID3D12PipelineState* GraphicsPipeline::Get() const
{
    return m_State.Get();
}

}
}
}

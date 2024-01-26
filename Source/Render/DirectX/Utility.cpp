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

#include "Utility.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{
namespace Utility
{

D3D12_HEAP_PROPERTIES MakeHeapProperties(D3D12_HEAP_TYPE Type)
{
    D3D12_HEAP_PROPERTIES Result { 0 };
    Result.Type = Type;
    Result.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    Result.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    Result.CreationNodeMask = 1;
    Result.VisibleNodeMask = 1;
    return Result;
}

D3D12_RESOURCE_DESC MakeResourceDescription(D3D12_RESOURCE_DIMENSION Dimension, DXGI_FORMAT Format)
{
    D3D12_RESOURCE_DESC Result;
    Result.Dimension = Dimension;
    Result.Alignment = 0;
    Result.Width = 0;
    Result.Height = 1;
    Result.DepthOrArraySize = 1;
    Result.MipLevels = 1;
    Result.Format = Format;
    Result.SampleDesc.Count = 1;
    Result.SampleDesc.Quality = 0;
    Result.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    Result.Flags = D3D12_RESOURCE_FLAG_NONE;
    return Result;
}

D3D12_RESOURCE_BARRIER MakeResourceBarrierTransition(ID3D12Resource* Resource,
    D3D12_RESOURCE_STATES Before,
    D3D12_RESOURCE_STATES After,
    UINT Subresource,
    D3D12_RESOURCE_BARRIER_FLAGS Flags)
{
    D3D12_RESOURCE_BARRIER Result { 0 };
    Result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Result.Transition.StateBefore = Before;
    Result.Transition.StateAfter = After;
    Result.Transition.pResource = Resource;
    Result.Transition.Subresource = Subresource;
    Result.Flags = Flags;
    return Result;
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC MakeDefaultGraphicsPipelineState()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC Result { 0 };
    Result.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    Result.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    Result.RasterizerState.FrontCounterClockwise = TRUE;
    Result.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    Result.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    Result.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    Result.RasterizerState.DepthClipEnable = TRUE;
    Result.RasterizerState.MultisampleEnable = FALSE;
    Result.RasterizerState.AntialiasedLineEnable = FALSE;
    Result.RasterizerState.ForcedSampleCount = 0;
    Result.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    Result.BlendState.AlphaToCoverageEnable = FALSE;
    Result.BlendState.IndependentBlendEnable = FALSE;

    for (UINT I = 0; I < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++I)
    {
        Result.BlendState.RenderTarget[I].BlendEnable = FALSE;
        Result.BlendState.RenderTarget[I].LogicOpEnable = FALSE;
        Result.BlendState.RenderTarget[I].SrcBlend = D3D12_BLEND_ONE;
        Result.BlendState.RenderTarget[I].DestBlend = D3D12_BLEND_ZERO;
        Result.BlendState.RenderTarget[I].BlendOp = D3D12_BLEND_OP_ADD;
        Result.BlendState.RenderTarget[I].SrcBlendAlpha = D3D12_BLEND_ONE;
        Result.BlendState.RenderTarget[I].DestBlendAlpha = D3D12_BLEND_ZERO;
        Result.BlendState.RenderTarget[I].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        Result.BlendState.RenderTarget[I].LogicOp = D3D12_LOGIC_OP_NOOP;
        Result.BlendState.RenderTarget[I].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    Result.DepthStencilState.DepthEnable = FALSE;
    Result.DepthStencilState.StencilEnable = FALSE;
    Result.SampleMask = UINT_MAX;
    Result.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    Result.NumRenderTargets = 1;
    Result.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    Result.SampleDesc.Count = 1;

    return Result;
}

D3D12_DEPTH_STENCIL_DESC MakeDepthStencilDescription()
{
    D3D12_DEPTH_STENCIL_DESC Result;
    Result.DepthEnable = TRUE;
    Result.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    Result.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    Result.StencilEnable = FALSE;
    Result.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    Result.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = { D3D12_STENCIL_OP_KEEP,
        D3D12_STENCIL_OP_KEEP,
        D3D12_STENCIL_OP_KEEP,
        D3D12_COMPARISON_FUNC_ALWAYS };
    Result.FrontFace = defaultStencilOp;
    Result.BackFace = defaultStencilOp;
    return Result;
}

D3D12_DESCRIPTOR_RANGE1 MakeDescriptorRange1(D3D12_DESCRIPTOR_RANGE_TYPE Type, D3D12_DESCRIPTOR_RANGE_FLAGS Flags)
{
    D3D12_DESCRIPTOR_RANGE1 Result;
    Result.RangeType = Type;
    Result.NumDescriptors = 1;
    Result.BaseShaderRegister = 0;
    Result.RegisterSpace = 0;
    Result.Flags = Flags;
    Result.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    return Result;
}

}
}
}
}

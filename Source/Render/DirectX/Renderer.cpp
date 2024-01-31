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

// Need to include before including any Windows headers.
#include "../../External/OctaneGUI/OctaneGUI.h"

#if defined(DEBUG)
#include "InfoQueue.hpp"
#endif

#include "../../Core/Console.hpp"
#include "../../Core/Containers/Array.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "../../Core/Math/Vertex.hpp"
#include "../../Platform/Window.hpp"
#include "Adapter.hpp"
#include "CommandAllocator.hpp"
#include "CommandList.hpp"
#include "CommandQueue.hpp"
#include "DescriptorHeap.hpp"
#include "Device.hpp"
#include "Renderer.hpp"
#include "RootSignature.hpp"
#include "Shader.hpp"
#include "Utility.hpp"
#include "Viewport.hpp"

#include <cstdio>

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

static Array<u8> GenerateTexture(u32 Width, u32 Height)
{
    const u32 RowPitch { Width * 4 };
    const u32 CellPitch { RowPitch >> 3 };
    const u32 CellHeight { Width >> 3 };

    Array<u8> Result;
    Result.Resize(RowPitch * Height);

    u8* Data = Result.Data();
    for (u32 I = 0; I < Result.Size(); I += 4)
    {
        const u32 X { I % RowPitch };
        const u32 Y { I / RowPitch };
        const u32 U { X / CellPitch };
        const u32 V { Y / CellHeight };

        if (U % 2 == V % 2)
        {
            Data[I] = 0x00;
            Data[I + 1] = 0x00;
            Data[I + 2] = 0x00;
            Data[I + 3] = 0xFF;
        }
        else
        {
            Data[I] = 0xFF;
            Data[I + 1] = 0xFF;
            Data[I + 2] = 0xFF;
            Data[I + 3] = 0xFF;
        }
    }

    return Result;
}

static Matrix4f Perspective(Platform::Window* Window)
{
    return Core::Math::PerspectiveMatrixLH(45.0f, Window->AspectRatio(), 0.1f, 100.f).Transpose();
}

static Matrix4f Orthographic(Platform::Window* Window)
{
    const Rectf Bounds { 0.0f, 0.0f, static_cast<f32>(Window->Size().X), static_cast<f32>(Window->Size().Y) };
    return Core::Math::OrthographicMatrixRH(Bounds, -1.0f, 1.0f).Transpose();
}

Renderer::Renderer()
    : LevelSketch::Render::Renderer()
{
}

bool Renderer::Initialize()
{
    m_Device = UniquePtr<Device>::New();

    if (!m_Device->Initialize())
    {
        return false;
    }

#if defined(DEBUG)
    InfoQueue::Initialize(m_Device->Get());
#endif

    m_Device->GetAdapter()->FillSummary(SummaryMut());
    m_ConstantBufferIndex = m_Device->MaxSRVDescriptors() - 1;

    return true;
}

bool Renderer::Initialize(Platform::Window* Window)
{
    const bool FirstWindow { m_Viewports.Size() == 0 };

    UniquePtr<Viewport> NewViewport { UniquePtr<Viewport>::New() };
    if (!NewViewport->Initialize(Window, m_Device.Get(), 2))
    {
        return false;
    }
    m_Viewports.Push(std::move(NewViewport));

    if (FirstWindow)
    {
        if (!LoadAssets(Window))
        {
            return false;
        }

        SetInitialized(true);
    }

    return true;
}

void Renderer::Shutdown()
{
}

static Vertex3 Vertices[3];

void Renderer::Render(Platform::Window* Window)
{
    if (!ResetCommands())
    {
        return;
    }

    Viewport* Viewport_ { GetViewportFor(Window) };

    ID3D12GraphicsCommandList1* CommandList { m_Device->GetCommandList()->Get() };
    CommandList->SetGraphicsRootSignature(m_Device->GetRootSignature()->Get());

    ID3D12DescriptorHeap* Heaps[] = { m_Device->SRVHeap()->Get() };
    CommandList->SetDescriptorHeaps(_countof(Heaps), Heaps);
    // TODO: Apply an offset to select a specific resource.
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDesc { m_Device->SRVHeap()->GPUOffset(0) };
    GPUDesc.ptr += GetTextureOffset(m_DefaultTexture);
    CommandList->SetGraphicsRootDescriptorTable(0, GPUDesc);

    const Core::Math::Vector2i Size { Window->Size() };
    D3D12_VIEWPORT View { 0.0f, 0.0f, (FLOAT)Size.X, (FLOAT)Size.Y, 0.0f, 1.0f };
    D3D12_RECT Scissor { 0, 0, (LONG)Size.X, (LONG)Size.Y };

    CommandList->RSSetViewports(1, &View);
    CommandList->RSSetScissorRects(1, &Scissor);

    D3D12_RESOURCE_BARRIER Barrier { Utility::MakeResourceBarrierTransition(Viewport_->CurrentRenderTarget(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET) };
    CommandList->ResourceBarrier(1, &Barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE RTVCPUDesc { Viewport_->RTVCPUOffset() };
    D3D12_CPU_DESCRIPTOR_HANDLE DSVCPUDesc { m_Device->DSVHeap()->CPUOffset(0) };
    CommandList->OMSetRenderTargets(1, &RTVCPUDesc, FALSE, &DSVCPUDesc);

    // Begin World rendering
    CommandList->SetPipelineState(m_PipelineState.Get());

    const float ClearColor[] { 0.0f, 0.2f, 0.4f, 1.0f };
    CommandList->ClearRenderTargetView(RTVCPUDesc, ClearColor, 0, nullptr);
    CommandList->ClearDepthStencilView(DSVCPUDesc, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    GPUDesc = m_Device->SRVHeap()->GPUOffset(m_ConstantBufferIndex);
    CommandList->SetGraphicsRootDescriptorTable(1, GPUDesc);

    m_ConstantBufferData.View = Matrix4f::LookAtLH({ 0.0f, 0.0f, -5.0f }, {}, { 0.0f, 1.0f, 0.0f }).Transpose();
    m_ConstantBufferData.Perspective = Perspective(Window);
    m_ConstantBufferData.Orthographic = Orthographic(Window);
    std::memcpy(m_ConstantBufferAddress, &m_ConstantBufferData, sizeof(m_ConstantBufferData));

    CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_RenderBuffer.BindViews(CommandList);
    CommandList->DrawIndexedInstanced(3, 1, 0, 0, 0);

    // Begin GUI rendering
    CommandList->SetPipelineState(m_PipelineStateGUI.Get());
    CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_RenderBufferGUI.BindViews(CommandList);

    for (const OctaneGUI::DrawCommand& Command : m_GUICommands)
    {
        GPUDesc = m_Device->SRVHeap()->GPUOffset(0);

        if (Command.TextureID() == 0)
        {
            GPUDesc.ptr += GetTextureOffset(m_WhiteTexture);
        }
        else
        {
            GPUDesc.ptr += GetTextureOffset(Command.TextureID());
        }

        D3D12_RECT Clip { Scissor };
        if (!Command.Clip().IsZero())
        {
            Clip.left = static_cast<LONG>(Command.Clip().Min.X);
            Clip.top = static_cast<LONG>(Command.Clip().Min.Y);
            Clip.right = static_cast<LONG>(Command.Clip().Max.X);
            Clip.bottom = static_cast<LONG>(Command.Clip().Max.Y);
        };

        CommandList->RSSetScissorRects(1, &Clip);
        CommandList->SetGraphicsRootDescriptorTable(0, GPUDesc);
        CommandList->DrawIndexedInstanced(Command.IndexCount(), 1, Command.IndexOffset(), Command.VertexOffset(), 0);
    }

    Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    CommandList->ResourceBarrier(1, &Barrier);

    ExecuteCommands();

    Viewport_->Present(1, 0);
    WaitForPreviousFrame();
    Viewport_->UpdateFrameIndex();

#if defined(DEBUG)
    InfoQueue::Poll();
#endif
}

u32 Renderer::LoadTexture(const void* Data, u32 Width, u32 Height, u8)
{
    if (!ResetCommands())
    {
        Core::Console::Warning("Failed to load texture! Command list could not be reset.");
        return 0;
    }

    Texture Tex;
    if (!Tex.Create(m_Device->Get(), Width, Height))
    {
        return 0;
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
    if (!Tex.Upload(m_Device->GetCommandList()->Get(),
            m_Device->SRVHeap()->Get(),
            m_Textures.Size() * m_Device->SRVHeap()->DescriptorSize(),
            Data,
            UploadResource))
    {
        return 0;
    }

    ExecuteCommands();
    WaitForPreviousFrame();

    m_Textures.Push(Tex);

    return Tex.ID();
}

void Renderer::UploadGUIData(OctaneGUI::Window*, const OctaneGUI::VertexBuffer& Buffer)
{
    if (!m_RenderBufferGUI.Initialized())
    {
        return;
    }

    if (!ResetCommands())
    {
        return;
    }

    m_RenderBufferGUI.UploadVertexData(Buffer.GetVertices().data(),
        static_cast<u64>(Buffer.GetVertexCount()) * sizeof(OctaneGUI::Vertex));
    m_RenderBufferGUI.UploadIndexData(Buffer.GetIndices().data(),
        static_cast<u64>(Buffer.GetIndexCount()) * sizeof(u32));

    ExecuteCommands();
    WaitForPreviousFrame();

    m_GUICommands.Clear().Reserve(Buffer.Commands().size());

    for (const OctaneGUI::DrawCommand& Command : Buffer.Commands())
    {
        m_GUICommands.Push(Command);
    }
}

bool Renderer::LoadAssets(Platform::Window* Window)
{
    u32 CompileFlags { 0 };

#if defined(_DEBUG)
    CompileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Shader VertexShader { Shader::LoadFromFile("Content/Shaders/HLSL/TestVS.hlsl") };
    bool CompileResult =
        VertexShader.SetName("DefaultVS")
            .SetEntryPoint("Main")
            .SetTarget("vs_5_0")
            .SetCompileFlags(CompileFlags)
            .AddInputElement(
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 })
            .AddInputElement(
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 })
            .AddInputElement(
                { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 })
            .Compile();
    if (!CompileResult)
    {
        Core::Console::Warning("Failed to compile vertex shader:\n%s",
            (LPCSTR)VertexShader.Errors()->GetBufferPointer());
        return false;
    }

    Shader PixelShader { Shader::LoadFromFile("Content/Shaders/HLSL/TestPS.hlsl") };
    PixelShader.SetName("DefaultPS").SetEntryPoint("Main").SetTarget("ps_5_0").SetCompileFlags(CompileFlags);
    CompileResult = PixelShader.Compile();
    if (!CompileResult)
    {
        Core::Console::Warning("Failed to compile pixel shader:\n%s", (LPCSTR)PixelShader.Errors()->GetBufferPointer());
        return false;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsDesc { Utility::MakeDefaultGraphicsPipelineState() };
    GraphicsDesc.InputLayout = { VertexShader.InputElements(), VertexShader.NumInputElements() };
    GraphicsDesc.pRootSignature = m_Device->GetRootSignature()->Get();
    GraphicsDesc.VS = { VertexShader.Blob()->GetBufferPointer(), VertexShader.Blob()->GetBufferSize() };
    GraphicsDesc.PS = { PixelShader.Blob()->GetBufferPointer(), PixelShader.Blob()->GetBufferSize() };
    GraphicsDesc.DepthStencilState = Utility::MakeDepthStencilDescription();
    GraphicsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    if (m_Device->Get()->CreateGraphicsPipelineState(&GraphicsDesc, IID_PPV_ARGS(&m_PipelineState)) != S_OK)
    {
        Core::Console::WriteLine("Failed to create default graphics pipeline state!");
        return false;
    }

    // Begin creation of GUI pipeline state.
    VertexShader = Shader::LoadFromFile("Content/Shaders/HLSL/GUIVS.hlsl");
    VertexShader.SetName("GUIVS")
        .SetEntryPoint("Main")
        .SetTarget("vs_5_0")
        .SetCompileFlags(CompileFlags)
        .AddInputElement({ "POSITION",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0 })
        .AddInputElement({ "TEXCOORD",
            0,
            DXGI_FORMAT_R32G32_FLOAT,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0 })
        .AddInputElement({ "COLOR",
            0,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            0,
            D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0 });
    CompileResult = VertexShader.Compile();
    if (!CompileResult)
    {
        Core::Console::Warning("Failed to compile GUI vertex shader:\n%s",
            (LPCSTR)VertexShader.Errors()->GetBufferPointer());
        return false;
    }

    PixelShader = Shader::LoadFromFile("Content/Shaders/HLSL/GUIPS.hlsl");
    CompileResult =
        PixelShader.SetName("GUIPS").SetEntryPoint("Main").SetTarget("ps_5_0").SetCompileFlags(CompileFlags).Compile();
    if (!CompileResult)
    {
        Core::Console::Warning("Failed to compile GUI pixel shader:\n%s",
            (LPCSTR)PixelShader.Errors()->GetBufferPointer());
        return false;
    }

    GraphicsDesc.InputLayout = { VertexShader.InputElements(), VertexShader.NumInputElements() };
    GraphicsDesc.VS = { VertexShader.Blob()->GetBufferPointer(), VertexShader.Blob()->GetBufferSize() };
    GraphicsDesc.PS = { PixelShader.Blob()->GetBufferPointer(), PixelShader.Blob()->GetBufferSize() };
    GraphicsDesc.DepthStencilState.DepthEnable = FALSE;
    GraphicsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    GraphicsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    GraphicsDesc.DepthStencilState.StencilEnable = FALSE;
    GraphicsDesc.DepthStencilState.FrontFace.StencilFailOp =
        GraphicsDesc.DepthStencilState.FrontFace.StencilDepthFailOp =
            GraphicsDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    GraphicsDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    GraphicsDesc.DepthStencilState.BackFace = GraphicsDesc.DepthStencilState.FrontFace;
    GraphicsDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

    GraphicsDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
    GraphicsDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    GraphicsDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    GraphicsDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    GraphicsDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    GraphicsDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    GraphicsDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    GraphicsDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    if (m_Device->Get()->CreateGraphicsPipelineState(&GraphicsDesc, IID_PPV_ARGS(&m_PipelineStateGUI)) != S_OK)
    {
        Core::Console::WriteLine("Failed to create GUI graphics pipeline state!");
        return false;
    }

    if (m_Device->Get()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)) != S_OK)
    {
        printf("Failed to create fence!\n");
        return false;
    }
    m_FenceValue = 1;

    m_FenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (m_FenceEvent == nullptr)
    {
        printf("Failed to create fence event!\n");
        return false;
    }

    m_RenderBuffer.Initialize(m_Device->Get(), 1000, 1000);

    // Vertex Buffer
    {
        const float Offset { 1.0f };
        Vertices[0] = { { 0.0f, Offset, 0.0f }, { 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } };
        Vertices[1] = { { -Offset, -Offset, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } };
        Vertices[2] = { { Offset, -Offset, 0.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } };

        m_RenderBuffer.SetStride(sizeof(Vertex3)).UploadVertexData(Vertices, sizeof(Vertices));
    }

    // Index Buffer
    {
        const u32 IndexBufferData[] = { 0, 1, 2 };
        const u32 IndexBufferSize = sizeof(IndexBufferData);

        m_RenderBuffer.SetFormat(DXGI_FORMAT_R32_UINT).UploadIndexData(IndexBufferData, IndexBufferSize);
    }

    // Depth Stencil View
    {
        D3D12_HEAP_PROPERTIES HeapProps { Utility::MakeHeapProperties() };
        D3D12_RESOURCE_DESC ResourceDesc { Utility::MakeResourceDescription(D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            DXGI_FORMAT_D32_FLOAT) };
        ResourceDesc.Width = Window->Size().X;
        ResourceDesc.Height = Window->Size().Y;
        ResourceDesc.DepthOrArraySize = 1;
        ResourceDesc.MipLevels = 0;
        ResourceDesc.SampleDesc.Count = 1;
        ResourceDesc.SampleDesc.Quality = 0;
        ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc { 0 };
        DSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
        DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        DSVDesc.Flags = D3D12_DSV_FLAG_NONE;

        D3D12_CLEAR_VALUE ClearValue { 0 };
        ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
        ClearValue.DepthStencil.Depth = 1.0f;
        ClearValue.DepthStencil.Stencil = 0;

        if (m_Device->Get()->CreateCommittedResource(&HeapProps,
                D3D12_HEAP_FLAG_NONE,
                &ResourceDesc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &ClearValue,
                IID_PPV_ARGS(&m_DepthStencil)) != S_OK)
        {
            Core::Console::Error("Failed to create depth stencil resource.");
            return false;
        }

        m_Device->Get()->CreateDepthStencilView(m_DepthStencil.Get(), &DSVDesc, m_Device->DSVHeap()->CPUOffset(0));
    }

    m_RenderBufferGUI.Initialize(m_Device->Get(), 100000, 100000);
    m_RenderBufferGUI.SetStride(sizeof(OctaneGUI::Vertex)).SetFormat(DXGI_FORMAT_R32_UINT);

    // Constant Buffer
    {
        D3D12_HEAP_PROPERTIES HeapProps { Utility::MakeHeapProperties(D3D12_HEAP_TYPE_UPLOAD) };
        D3D12_RESOURCE_DESC ResourceDesc { Utility::MakeResourceDescription() };
        ResourceDesc.Width = sizeof(ConstantBufferData);
        ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        if (m_Device->Get()->CreateCommittedResource(&HeapProps,
                D3D12_HEAP_FLAG_NONE,
                &ResourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_ConstantBuffer)) != S_OK)
        {
            Core::Console::Error("Failed to create constant buffer.");
            return false;
        }

        D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBufferView { 0 };
        ConstantBufferView.BufferLocation = m_ConstantBuffer->GetGPUVirtualAddress();
        ConstantBufferView.SizeInBytes = sizeof(ConstantBufferData);

        D3D12_CPU_DESCRIPTOR_HANDLE CPUDesc { m_Device->SRVHeap()->CPUOffset(m_ConstantBufferIndex) };
        m_Device->Get()->CreateConstantBufferView(&ConstantBufferView, CPUDesc);

        D3D12_RANGE Range { 0, 0 };
        if (m_ConstantBuffer->Map(0, &Range, reinterpret_cast<void**>(&m_ConstantBufferAddress)) != S_OK)
        {
            Core::Console::Error("Failed to map constant buffer memory.");
            return false;
        }
    }

    // Upload Vertex/Index buffers. The Load texture function will reset the command list.
    // Want to avoid resetting the command list while there are queued commands.
    ExecuteCommands();
    WaitForPreviousFrame();

    // Texture
    {
        const u8 WhiteTexture[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
        m_WhiteTexture = LoadTexture(WhiteTexture, 1, 1);

        const u32 Width { 256 };
        const u32 Height { 256 };
        const Array<u8> Data { GenerateTexture(Width, Height) };
        m_DefaultTexture = LoadTexture(Data.Data(), Width, Height, 4);
    }

    WaitForPreviousFrame();

    return true;
}

void Renderer::WaitForPreviousFrame()
{
    const UINT64 Fence { m_FenceValue };
    if (!m_Device->GetCommandQueue()->Signal(m_Fence.Get(), Fence))
    {
        return;
    }
    m_FenceValue++;

    if (m_Fence->GetCompletedValue() < Fence)
    {
        if (m_Fence->SetEventOnCompletion(Fence, m_FenceEvent) == S_OK)
        {
            WaitForSingleObject(m_FenceEvent, INFINITE);
        }
        else
        {
            printf("Failed to set event on completion!\n");
        }
    }
}

bool Renderer::ExecuteCommands()
{
    if (!m_Device->GetCommandList()->Close())
    {
        return false;
    }

    ID3D12CommandList* CommandLists[] { m_Device->GetCommandList()->Get() };
    m_Device->GetCommandQueue()->Execute(_countof(CommandLists), CommandLists);

    return true;
}

bool Renderer::ResetCommands()
{
    if (!m_Device->GetCommandAllocator()->Reset())
    {
        return false;
    }

    if (!m_Device->GetCommandList()->Reset())
    {
        return false;
    }

    return true;
}

u64 Renderer::GetTextureOffset(u32 ID) const
{
    for (const Texture& Tex : m_Textures)
    {
        if (Tex.ID() == ID)
        {
            return Tex.Offset();
        }
    }

    return 0;
}

Viewport* Renderer::GetViewportFor(Platform::Window* Window) const
{
    for (const UniquePtr<Viewport>& Item : m_Viewports)
    {
        if (Item->GetWindow() == Window)
        {
            return Item.Get();
        }
    }

    return nullptr;
}

}
}
}

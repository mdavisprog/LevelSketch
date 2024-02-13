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

#if defined(DEBUG)
#include "InfoQueue.hpp"
#endif

#include "../../Core/Console.hpp"
#include "../../Core/Containers/Array.hpp"
#include "../../Core/Math/Color.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "../../Core/Math/Vertex.hpp"
#include "../../Platform/FileSystem.hpp"
#include "../../Platform/Window.hpp"
#include "../VertexBufferDescription.hpp"
#include "../VertexDataDescription.hpp"
#include "../ViewportRect.hpp"
#include "Adapter.hpp"
#include "CommandAllocator.hpp"
#include "CommandList.hpp"
#include "CommandQueue.hpp"
#include "DepthStencil.hpp"
#include "DescriptorHeap.hpp"
#include "Device.hpp"
#include "GraphicsPipeline.hpp"
#include "Renderer.hpp"
#include "RootSignature.hpp"
#include "Shader.hpp"
#include "Utility.hpp"
#include "VertexBuffer.hpp"
#include "Viewport.hpp"

#include <cstdio>

namespace LevelSketch
{
namespace Render
{

String Renderer::ShadersDirectory()
{
    return Platform::FileSystem::CombinePaths(Platform::FileSystem::ShadersDirectory(), "HLSL");
}

namespace DirectX
{

static Matrix4f Orthographic(Platform::Window* Window)
{
    const Rectf Bounds { 0.0f, 0.0f, static_cast<f32>(Window->Size().X), static_cast<f32>(Window->Size().Y) };
    return Core::Math::OrthographicMatrixLH(Bounds, -1.0f, 1.0f);
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

    if (!LoadAssets())
    {
        return false;
    }

    return true;
}

bool Renderer::Initialize(Platform::Window* Window)
{
    UniquePtr<Viewport> NewViewport { UniquePtr<Viewport>::New() };
    if (!NewViewport->Initialize(Window, m_Device.Get(), 2))
    {
        return false;
    }
    m_Viewports.Push(std::move(NewViewport));

    if (m_DepthStencil == nullptr)
    {
        m_DepthStencil = UniquePtr<DepthStencil>::New();
        if (!m_DepthStencil->Initialize(m_Device.Get(), Window->Size()))
        {
            return false;
        }
    }

    return true;
}

void Renderer::Shutdown()
{
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

bool Renderer::BindTexture(u32 ID)
{
    if (ID == 0)
    {
        Core::Console::Warning("Invalid texture given to BindTexture.");
        return false;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GPUDesc { m_Device->SRVHeap()->GPUOffset(0) };
    GPUDesc.ptr += GetTextureOffset(ID);
    m_Device->GetCommandList()->Get()->SetGraphicsRootDescriptorTable(0, GPUDesc);

    return true;
}

bool Renderer::BeginRender(Platform::Window* Window, const Colorf& ClearColor)
{
    if (!ResetCommands())
    {
        return false;
    }

    Viewport* Viewport_ { GetViewportFor(Window) };

    if (Viewport_ == nullptr)
    {
        return false;
    }

    ID3D12GraphicsCommandList1* CommandList { m_Device->GetCommandList()->Get() };
    CommandList->SetGraphicsRootSignature(m_Device->GetRootSignature()->Get());

    ID3D12DescriptorHeap* Heaps[] = { m_Device->SRVHeap()->Get() };
    CommandList->SetDescriptorHeaps(_countof(Heaps), Heaps);

    const Core::Math::Vector2i Size { Window->Size() };
    D3D12_VIEWPORT View { 0.0f, 0.0f, (FLOAT)Size.X, (FLOAT)Size.Y, 0.0f, 1.0f };
    D3D12_RECT Scissor { 0, 0, (LONG)Size.X, (LONG)Size.Y };

    CommandList->RSSetViewports(1, &View);
    CommandList->RSSetScissorRects(1, &Scissor);

    D3D12_RESOURCE_BARRIER Barrier {};
    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    Barrier.Transition.pResource = Viewport_->CurrentRenderTarget();
    Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    CommandList->ResourceBarrier(1, &Barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE RTVCPUDesc { Viewport_->RTVCPUOffset() };
    D3D12_CPU_DESCRIPTOR_HANDLE DSVCPUDesc { m_DepthStencil->CPUOffset() };
    CommandList->OMSetRenderTargets(1, &RTVCPUDesc, FALSE, &DSVCPUDesc);

    const float Color[] { ClearColor.R, ClearColor.G, ClearColor.B, ClearColor.A };
    CommandList->ClearRenderTargetView(RTVCPUDesc, Color, 0, nullptr);
    CommandList->ClearDepthStencilView(DSVCPUDesc, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    D3D12_GPU_DESCRIPTOR_HANDLE GPUDesc = m_Device->SRVHeap()->GPUOffset(m_ConstantBufferIndex);
    CommandList->SetGraphicsRootDescriptorTable(1, GPUDesc);

    m_ConstantBufferData.Perspective = Core::Math::PerspectiveMatrixLH(45.0f, Window->AspectRatio(), 0.1f, 100.0f);
    m_ConstantBufferData.Orthographic = Orthographic(Window);
    std::memcpy(m_ConstantBufferAddress, &m_ConstantBufferData, sizeof(m_ConstantBufferData));

    CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return true;
}

void Renderer::EndRender(Platform::Window* Window)
{
    Viewport* Viewport_ { GetViewportFor(Window) };

    if (Viewport_ == nullptr)
    {
        return;
    }

    D3D12_RESOURCE_BARRIER Barrier {};
    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    Barrier.Transition.pResource = Viewport_->CurrentRenderTarget();
    Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    m_Device->GetCommandList()->Get()->ResourceBarrier(1, &Barrier);

    ExecuteCommands();

    Viewport_->Present(1, 0);
    WaitForPreviousFrame();
    Viewport_->UpdateFrameIndex();

#if defined(DEBUG)
    InfoQueue::Poll();
#endif
}

void Renderer::SetViewportRect(const ViewportRect& Rect)
{
    D3D12_VIEWPORT Viewport { Rect.Bounds.X,
        Rect.Bounds.Y,
        Rect.Bounds.W,
        Rect.Bounds.H,
        Rect.MinDepth,
        Rect.MaxDepth };

    m_Device->GetCommandList()->Get()->RSSetViewports(1, &Viewport);
}

void Renderer::SetScissor(const Recti& Rect)
{
    D3D12_RECT Scissor { Rect.Left(), Rect.Top(), Rect.Right(), Rect.Bottom() };

    m_Device->GetCommandList()->Get()->RSSetScissorRects(1, &Scissor);
}

u32 Renderer::CreateGraphicsPipeline(const GraphicsPipelineDescription& Description)
{
    UniquePtr<GraphicsPipeline> Pipeline { UniquePtr<GraphicsPipeline>::New() };

    if (!Pipeline->Initialize(m_Device.Get(), Description))
    {
        return 0;
    }

    const u32 Result { Pipeline->ID() };
    m_GraphicsPipelines.Push(std::move(Pipeline));
    return Result;
}

bool Renderer::BindGraphicsPipeline(u32 ID)
{
    if (ID == 0)
    {
        Core::Console::Warning("Invalid graphic pipeline ID given to BindGraphicsPipeline.");
        return false;
    }

    GraphicsPipeline* Pipeline { GetGraphicsPipeline(ID) };

    if (Pipeline == nullptr)
    {
        return false;
    }

    m_Device->GetCommandList()->Get()->SetPipelineState(Pipeline->Get());

    return true;
}

void Renderer::DrawIndexed(u32 IndexCount, u32 InstanceCount, u32 StartIndex, u32 BaseVertex, u32 StartInstance)
{
    m_Device->GetCommandList()->Get()->DrawIndexedInstanced(IndexCount,
        InstanceCount,
        StartIndex,
        BaseVertex,
        StartInstance);
}

u32 Renderer::CreateVertexBuffer(const VertexBufferDescription& Description)
{
    UniquePtr<VertexBuffer> Buffer { UniquePtr<VertexBuffer>::New() };

    if (!Buffer->Initialize(m_Device.Get(), Description))
    {
        return 0;
    }

    const u32 Result { Buffer->ID() };
    m_VertexBuffers.Push(std::move(Buffer));
    return Result;
}

bool Renderer::UploadVertexData(u32 ID, const VertexDataDescription& Description)
{
    if (!ResetCommands())
    {
        return false;
    }

    if (ID == 0)
    {
        Core::Console::Warning("Invalid vertex buffer ID '%d' given to UploadVertexData.", ID);
        return false;
    }

    VertexBuffer* Buffer { GetVertexBuffer(ID) };

    if (Buffer == nullptr)
    {
        return false;
    }

    if (!Buffer->UploadVertexData(Description.VertexData, Description.VertexDataSize))
    {
        return false;
    }

    if (!Buffer->UploadIndexData(Description.IndexData, Description.IndexDataSize))
    {
        return false;
    }

    if (!ExecuteCommands())
    {
        return false;
    }

    WaitForPreviousFrame();

    return true;
}

bool Renderer::BindVertexBuffer(u32 ID)
{
    if (ID == 0)
    {
        Core::Console::Warning("Invalid vertex buffer ID '%d' given to BindVertexBuffer.", ID);
        return false;
    }

    VertexBuffer* Buffer { GetVertexBuffer(ID) };

    if (Buffer == nullptr)
    {
        return false;
    }

    Buffer->BindViews(m_Device->GetCommandList()->Get());

    return true;
}

void Renderer::UpdateViewMatrix(const Matrix4f& View)
{
    m_ConstantBufferData.View = View;
}

bool Renderer::LoadAssets()
{
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

    // TODO: No longer needed once constant buffers are created outside of initialization.
    ExecuteCommands();
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

VertexBuffer* Renderer::GetVertexBuffer(u32 ID) const
{
    VertexBuffer* Result { nullptr };

    for (const UniquePtr<VertexBuffer>& Buffer : m_VertexBuffers)
    {
        if (Buffer->ID() == ID)
        {
            Result = Buffer.Get();
            break;
        }
    }

    if (Result == nullptr)
    {
        Core::Console::Warning("Failed to find vertex buffer with ID '%d'.", ID);
    }

    return Result;
}

GraphicsPipeline* Renderer::GetGraphicsPipeline(u32 ID) const
{
    GraphicsPipeline* Result { nullptr };

    for (const UniquePtr<GraphicsPipeline>& Pipeline : m_GraphicsPipelines)
    {
        if (Pipeline->ID() == ID)
        {
            Result = Pipeline.Get();
            break;
        }
    }

    if (Result == nullptr)
    {
        Core::Console::Warning("Failed to find graphics pipeline with ID '%d'.", ID);
    }

    return Result;
}

}
}
}

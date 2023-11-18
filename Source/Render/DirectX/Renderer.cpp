/**

MIT License

Copyright (c) 2023 Mitchell Davis <mdavisprog@gmail.com>

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

#include "Renderer.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "../../Platform/Window.hpp"

#include <cstdio>

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

Renderer::Renderer()
    : LevelSketch::Render::Renderer()
{
}

bool Renderer::Initialize()
{
    if (!LoadPipeline(Window()))
    {
        return false;
    }

    if (!LoadAssets(Window()))
    {
        return false;
    }

    return true;
}

void Renderer::Shutdown()
{
}

void Renderer::Render()
{
    if (m_CommandAllocator->Reset() != S_OK)
    {
        printf("Failed to reset command allocator!\n");
        return;
    }

    if (m_CommandList->Reset(m_CommandAllocator.Get(), m_PipelineState.Get()) != S_OK)
    {
        printf("Failed to reset command list!\n");
        return;
    }

    const Core::Math::Vector2i Size { Window()->Size() };
    D3D12_VIEWPORT View { 0.0f, 0.0f, (FLOAT)Size.X, (FLOAT)Size.Y, 0.0f, 1.0f };
    D3D12_RECT Scissor { 0, 0, (LONG)Size.X, (LONG)Size.Y };

    m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
    m_CommandList->RSSetViewports(1, &View);
    m_CommandList->RSSetScissorRects(1, &Scissor);

    D3D12_RESOURCE_BARRIER Barrier;
    Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    Barrier.Transition.pResource = m_RenderTargets[m_FrameIndex].Get();
    Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_CommandList->ResourceBarrier(1, &Barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE CPUDesc { m_Heap->GetCPUDescriptorHandleForHeapStart() };
    CPUDesc.ptr = SIZE_T(INT64(CPUDesc.ptr) + INT64(m_FrameIndex) * INT64(m_HeapDescriptorSize));
    m_CommandList->OMSetRenderTargets(1, &CPUDesc, FALSE, nullptr);

    const float ClearColor[] { 0.0f, 0.2f, 0.4f, 1.0f };
    m_CommandList->ClearRenderTargetView(CPUDesc, ClearColor, 0, nullptr);
    m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    m_CommandList->DrawInstanced(3, 1, 0, 0);

    Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    m_CommandList->ResourceBarrier(1, &Barrier);

    if (m_CommandList->Close() != S_OK)
    {
        printf("Failed to close command list!\n");
    }

    ID3D12CommandList* CommandLists[] { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);

    if (m_SwapChain->Present(1, 0) != S_OK)
    {
        printf("Failed to present!\n");
    }

    WaitForPreviousFrame();
}

bool Renderer::LoadPipeline(Platform::Window* Window)
{
    Microsoft::WRL::ComPtr<IDXGIFactory4> Factory;
    if (CreateDXGIFactory2(0, IID_PPV_ARGS(&Factory)) != S_OK)
    {
        printf("Failed in CreateDXGIFactor2!\n");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter { GetHardwareAdapter(Factory.Get()) };
    if (D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)) != S_OK)
    {
        printf("Failed in D3D12CreateDevice!\n");
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC CommandQueueDescription { 0 };
    CommandQueueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    CommandQueueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (m_Device->CreateCommandQueue(&CommandQueueDescription, IID_PPV_ARGS(&m_CommandQueue)) != S_OK)
    {
        printf("Failed to create device command queue!\n");
        return false;
    }

    const Core::Math::Vector2i Size { Window->Size() };
    DXGI_SWAP_CHAIN_DESC1 SwapChainDescription { 0 };
    SwapChainDescription.BufferCount = FRAME_COUNT;
    SwapChainDescription.Width = (UINT)Size.X;
    SwapChainDescription.Height = (UINT)Size.Y;
    SwapChainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    SwapChainDescription.SampleDesc.Count = 1;

    const HWND Handle { reinterpret_cast<HWND>(Window->Handle()) };
    Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain;
    if (Factory->CreateSwapChainForHwnd(
        m_CommandQueue.Get(),
        Handle,
        &SwapChainDescription,
        nullptr,
        nullptr,
        &SwapChain
    ) != S_OK)
    {
        printf("Failed to create swap chain!\n");
        return false;
    }

    if (Factory->MakeWindowAssociation(Handle, DXGI_MWA_NO_ALT_ENTER) != S_OK)
    {
        printf("Failed to make window association!\n");
        return false;
    }

    if (SwapChain.As(&m_SwapChain) != S_OK)
    {
        printf("Failed to retrieve swap chain!\n");
        return false;
    }

    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

    D3D12_DESCRIPTOR_HEAP_DESC HeapDescription { 0 };
    HeapDescription.NumDescriptors = FRAME_COUNT;
    HeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    HeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (m_Device->CreateDescriptorHeap(&HeapDescription, IID_PPV_ARGS(&m_Heap)) != S_OK)
    {
        printf("Failed to create descriptor heap!\n");
        return false;
    }

    m_HeapDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle { m_Heap->GetCPUDescriptorHandleForHeapStart() };
    for (UINT I = 0; I < FRAME_COUNT; ++I)
    {
        if (m_SwapChain->GetBuffer(I, IID_PPV_ARGS(&m_RenderTargets[I])) != S_OK)
        {
            printf("Failed to get buffer %d for swap chain!\n", I);
            return false;
        }

        m_Device->CreateRenderTargetView(m_RenderTargets[I].Get(), nullptr, CPUHandle);
        CPUHandle.ptr = SIZE_T(INT64(CPUHandle.ptr) + INT64(1) * INT64(m_HeapDescriptorSize));
    }

    if (m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)) != S_OK)
    {
        printf("Failed to create command allocator!\n");
        return false;
    }

    return true;
}

bool Renderer::LoadAssets(Platform::Window* Window)
{
    D3D12_ROOT_SIGNATURE_DESC RootSignatureDescription { 0 };
    RootSignatureDescription.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> Signature;
    Microsoft::WRL::ComPtr<ID3DBlob> Error;
    if (D3D12SerializeRootSignature(&RootSignatureDescription, D3D_ROOT_SIGNATURE_VERSION_1, &Signature, &Error) != S_OK)
    {
        printf("Failed to serialize root signature!\n");
        return false;
    }

    if (m_Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)) != S_OK)
    {
        printf("Failed to create root signature!\n");
        return false;
    }

    const char* Source = R"(
        struct PSInput
        {
            float4 position : SV_POSITION;
            float4 color : COLOR;
        };

        PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
        {
            PSInput result;

            result.position = position;
            result.color = color;

            return result;
        }

        float4 PSMain(PSInput input) : SV_TARGET
        {
            return input.color;
        }
    )";

    Microsoft::WRL::ComPtr<ID3DBlob> VertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShader;

    const size_t SourceLength { strlen(Source) };
    if (D3DCompile(
        (LPVOID)Source,
        SourceLength,
        "Default",
        nullptr,
        nullptr,
        "VSMain",
        "vs_5_0",
        0,
        0,
        &VertexShader,
        nullptr
    ) != S_OK)
    {
        printf("Failed to compile vertex shader!\n");
        return false;
    }

    if (D3DCompile(
        (LPVOID)Source,
        SourceLength,
        "Default",
        nullptr,
        nullptr,
        "PSMain",
        "ps_5_0",
        0,
        0,
        &PixelShader,
        nullptr
    ) != S_OK)
    {
        printf("Failed to compile pixel shader!\n");
        return false;
    }

    const D3D12_INPUT_ELEMENT_DESC InputElements[]
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsDesc { 0 };
    GraphicsDesc.InputLayout = { InputElements, _countof(InputElements) };
    GraphicsDesc.pRootSignature = m_RootSignature.Get();
    GraphicsDesc.VS = { VertexShader->GetBufferPointer(), VertexShader->GetBufferSize() };
    GraphicsDesc.PS = { PixelShader->GetBufferPointer(), PixelShader->GetBufferSize() };
    GraphicsDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    GraphicsDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    GraphicsDesc.RasterizerState.FrontCounterClockwise = FALSE;
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

    GraphicsDesc.DepthStencilState.DepthEnable = FALSE;
    GraphicsDesc.DepthStencilState.StencilEnable = FALSE;
    GraphicsDesc.SampleMask = UINT_MAX;
    GraphicsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    GraphicsDesc.NumRenderTargets = 1;
    GraphicsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    GraphicsDesc.SampleDesc.Count = 1;

    if (m_Device->CreateGraphicsPipelineState(&GraphicsDesc, IID_PPV_ARGS(&m_PipelineState)) != S_OK)
    {
        printf("Failed to create graphics pipeline state!\n");
        return false;
    }

    if (m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), m_PipelineState.Get(), IID_PPV_ARGS(&m_CommandList)) != S_OK)
    {
        printf("Failed to create command list!\n");
        return false;
    }

    if (m_CommandList->Close() != S_OK)
    {
        printf("Failed to close command list!\n");
        return false;
    }

    const float AspectRatio { (float)Window->Size().X / (float)Window->Size().Y };
    const float Vertices[] =
    {
        0.0f, 0.25f * AspectRatio, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.25f, -0.25f * AspectRatio, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.25f, -0.25f * AspectRatio, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
    };
    const UINT VerticesSize { sizeof(Vertices) };

    D3D12_HEAP_PROPERTIES HeapProperties { 0 };
    HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    HeapProperties.CreationNodeMask = 1;
    HeapProperties.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC ResourceDesc { 0 };
    ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ResourceDesc.Width = VerticesSize;
    ResourceDesc.Height = 1;
    ResourceDesc.DepthOrArraySize = 1;
    ResourceDesc.MipLevels = 1;
    ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    ResourceDesc.SampleDesc.Count = 1;
    ResourceDesc.SampleDesc.Quality = 0;
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Not an efficient way to upload vertex data. Refer to other DX12 samples
    // for proper way of doing this.
    if (m_Device->CreateCommittedResource(
        &HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &ResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_VertexBuffer)
    ) != S_OK)
    {
        printf("Failed to create committed resource!\n");
        return false;
    }

    UINT8* VertexData { nullptr };
    D3D12_RANGE Range { 0, 0 };
    if (m_VertexBuffer->Map(0, &Range, reinterpret_cast<void**>(&VertexData)) != S_OK)
    {
        printf("Failed to map vertex buffer!\n");
        return false;
    }
    memcpy(VertexData, Vertices, sizeof(Vertices));
    m_VertexBuffer->Unmap(0, nullptr);

    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.StrideInBytes = sizeof(float) * 7; // 3 floats for position, 4 floats for color.
    m_VertexBufferView.SizeInBytes = VerticesSize;

    if (m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)) != S_OK)
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

    WaitForPreviousFrame();

    return true;
}

IDXGIAdapter1* Renderer::GetHardwareAdapter(IDXGIFactory1* Factory) const
{
    Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter;
    Microsoft::WRL::ComPtr<IDXGIFactory6> Factory6;
    if (Factory->QueryInterface(IID_PPV_ARGS(&Factory6)) == S_OK)
    {
        for (UINT I = 0; Factory6->EnumAdapterByGpuPreference(I, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&Adapter)) == S_OK; ++I)
        {
            DXGI_ADAPTER_DESC1 Description;
            Adapter->GetDesc1(&Description);

            if (Description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            if (D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr) == S_OK)
            {
                break;
            }
        }
    }

    if (Adapter.Get() == nullptr)
    {
        for (UINT I = 0; Factory->EnumAdapters1(I, &Adapter) == S_OK; ++I)
        {
            DXGI_ADAPTER_DESC1 Description;
            Adapter->GetDesc1(&Description);

            if (Description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            if (D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr) == S_OK)
            {
                break;
            }
        }
    }

    return Adapter.Detach();
}

void Renderer::WaitForPreviousFrame()
{
    const UINT64 Fence { m_FenceValue };
    if (m_CommandQueue->Signal(m_Fence.Get(), Fence) != S_OK)
    {
        printf("Failed to signal command queue!\n");
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

    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

}
}
}

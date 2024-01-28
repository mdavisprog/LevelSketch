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
#include "Renderer.hpp"
#include "Shader.hpp"
#include "Utility.hpp"

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

Renderer::Renderer()
    : LevelSketch::Render::Renderer()
{
}

bool Renderer::Initialize()
{
    return true;
}

bool Renderer::Initialize(Platform::Window* Window)
{
    if (m_Device != nullptr)
    {
        return true;
    }

    if (!LoadPipeline(Window))
    {
        return false;
    }

    if (!LoadAssets(Window))
    {
        return false;
    }

    SetInitialized(true);
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

    m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());

    ID3D12DescriptorHeap* Heaps[] = { m_SRVHeap.Get() };
    m_CommandList->SetDescriptorHeaps(_countof(Heaps), Heaps);
    // TODO: Apply an offset to select a specific resource.
    D3D12_GPU_DESCRIPTOR_HANDLE GPUDesc { m_SRVHeap->GetGPUDescriptorHandleForHeapStart() };
    GPUDesc.ptr += GetTextureOffset(m_DefaultTexture);
    m_CommandList->SetGraphicsRootDescriptorTable(0, GPUDesc);

    const Core::Math::Vector2i Size { Window->Size() };
    D3D12_VIEWPORT View { 0.0f, 0.0f, (FLOAT)Size.X, (FLOAT)Size.Y, 0.0f, 1.0f };
    D3D12_RECT Scissor { 0, 0, (LONG)Size.X, (LONG)Size.Y };

    m_CommandList->RSSetViewports(1, &View);
    m_CommandList->RSSetScissorRects(1, &Scissor);

    D3D12_RESOURCE_BARRIER Barrier { Utility::MakeResourceBarrierTransition(m_RenderTargets[m_FrameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET) };
    m_CommandList->ResourceBarrier(1, &Barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE RTVCPUDesc { m_RTVHeap->GetCPUDescriptorHandleForHeapStart() };
    D3D12_CPU_DESCRIPTOR_HANDLE DSVCPUDesc { m_DSVHeap->GetCPUDescriptorHandleForHeapStart() };
    RTVCPUDesc.ptr = SIZE_T(INT64(RTVCPUDesc.ptr) + INT64(m_FrameIndex) * INT64(m_HeapDescriptorSize));
    m_CommandList->OMSetRenderTargets(1, &RTVCPUDesc, FALSE, &DSVCPUDesc);

    // Begin World rendering
    m_CommandList->SetPipelineState(m_PipelineState.Get());

    const float ClearColor[] { 0.0f, 0.2f, 0.4f, 1.0f };
    m_CommandList->ClearRenderTargetView(RTVCPUDesc, ClearColor, 0, nullptr);
    m_CommandList->ClearDepthStencilView(DSVCPUDesc, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    GPUDesc = m_SRVHeap->GetGPUDescriptorHandleForHeapStart();
    GPUDesc.ptr += m_ConstantBufferTableOffset;
    m_CommandList->SetGraphicsRootDescriptorTable(1, GPUDesc);
    std::memcpy(m_ConstantBufferAddress, &m_ConstantBufferData, sizeof(m_ConstantBufferData));

    m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_RenderBuffer.BindViews(m_CommandList.Get());
    m_CommandList->DrawIndexedInstanced(3, 1, 0, 0, 0);

    // Begin GUI rendering
    m_CommandList->SetPipelineState(m_PipelineStateGUI.Get());
    m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_RenderBufferGUI.BindViews(m_CommandList.Get());

    for (const OctaneGUI::DrawCommand& Command : m_GUICommands)
    {
        GPUDesc = m_SRVHeap->GetGPUDescriptorHandleForHeapStart();

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

        m_CommandList->RSSetScissorRects(1, &Clip);
        m_CommandList->SetGraphicsRootDescriptorTable(0, GPUDesc);
        m_CommandList->DrawIndexedInstanced(Command.IndexCount(), 1, Command.IndexOffset(), Command.VertexOffset(), 0);
    }

    Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    m_CommandList->ResourceBarrier(1, &Barrier);

    ExecuteCommands();

    if (m_SwapChain->Present(1, 0) != S_OK)
    {
        printf("Failed to present!\n");
    }

    WaitForPreviousFrame();

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
    if (!Tex.Create(m_Device.Get(), Width, Height))
    {
        return 0;
    }

    Microsoft::WRL::ComPtr<ID3D12Resource> UploadResource;
    if (!Tex.Upload(m_CommandList.Get(),
            m_SRVHeap.Get(),
            m_Textures.Size() * m_SRVHeapDescriptorSize,
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

bool Renderer::LoadPipeline(Platform::Window* Window)
{
    Adapter Adapter_ {};
    if (!Adapter_.Initialize())
    {
        return false;
    }

    if (D3D12CreateDevice(Adapter_.GetAdapter(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)) != S_OK)
    {
        printf("Failed in D3D12CreateDevice!\n");
        return false;
    }

#if defined(DEBUG)
    InfoQueue::Initialize(m_Device.Get());
#endif

    Adapter_.FillSummary(SummaryMut());

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
    if (Adapter_.GetFactory()->CreateSwapChainForHwnd(m_CommandQueue.Get(),
            Handle,
            &SwapChainDescription,
            nullptr,
            nullptr,
            &SwapChain) != S_OK)
    {
        printf("Failed to create swap chain!\n");
        return false;
    }

    if (Adapter_.GetFactory()->MakeWindowAssociation(Handle, DXGI_MWA_NO_ALT_ENTER) != S_OK)
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

    D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc { 0 };
    RTVHeapDesc.NumDescriptors = FRAME_COUNT;
    RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (m_Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_RTVHeap)) != S_OK)
    {
        printf("Failed to create descriptor heap!\n");
        return false;
    }

    D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc { 0 };
    DSVHeapDesc.NumDescriptors = 1;
    DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (m_Device->CreateDescriptorHeap(&DSVHeapDesc, IID_PPV_ARGS(&m_DSVHeap)) != S_OK)
    {
        Core::Console::Error("Failed to create depth stencil view.");
        return false;
    }

    D3D12_DESCRIPTOR_HEAP_DESC SRVHeapDesc { 0 };
    SRVHeapDesc.NumDescriptors = MAX_DESCRIPTORS;
    SRVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    SRVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (m_Device->CreateDescriptorHeap(&SRVHeapDesc, IID_PPV_ARGS(&m_SRVHeap)) != S_OK)
    {
        Core::Console::WriteLine("Failed to create SRV descriptor heap.");
        return false;
    }

    m_HeapDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_SRVHeapDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle { m_RTVHeap->GetCPUDescriptorHandleForHeapStart() };
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
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE FeatureData { D3D_ROOT_SIGNATURE_VERSION_1_1 };
        if (m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &FeatureData, sizeof(FeatureData)) != S_OK)
        {
            Core::Console::WriteLine("Root signature version 1.1 is not supported.");
            FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        D3D12_DESCRIPTOR_RANGE1 DescRange[2] { Utility::MakeDescriptorRange1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                                                   D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC),
            Utility::MakeDescriptorRange1(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC) };

        D3D12_ROOT_PARAMETER1 RootParameters[2];
        RootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        RootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        RootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
        RootParameters[0].DescriptorTable.pDescriptorRanges = &DescRange[0];

        RootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        RootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
        RootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
        RootParameters[1].DescriptorTable.pDescriptorRanges = &DescRange[1];

        D3D12_STATIC_SAMPLER_DESC Sampler = { 0 };
        Sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        Sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        Sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        Sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        Sampler.MipLODBias = 0.0f;
        Sampler.MaxAnisotropy = 0;
        Sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        Sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        Sampler.MinLOD = 0.0f;
        Sampler.MaxLOD = D3D12_FLOAT32_MAX;
        Sampler.ShaderRegister = 0;
        Sampler.RegisterSpace = 0;
        Sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc { 0 };
        RootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
        RootSignatureDesc.Desc_1_1.NumParameters = _countof(RootParameters);
        RootSignatureDesc.Desc_1_1.pParameters = RootParameters;
        RootSignatureDesc.Desc_1_1.NumStaticSamplers = 1;
        RootSignatureDesc.Desc_1_1.pStaticSamplers = &Sampler;
        RootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        Microsoft::WRL::ComPtr<ID3DBlob> Signature;
        Microsoft::WRL::ComPtr<ID3DBlob> Error;
        if (D3D12SerializeVersionedRootSignature(&RootSignatureDesc, &Signature, &Error) != S_OK)
        {
            printf("Failed to serialize root signature!\n");
            return false;
        }

        if (m_Device->CreateRootSignature(0,
                Signature->GetBufferPointer(),
                Signature->GetBufferSize(),
                IID_PPV_ARGS(&m_RootSignature)) != S_OK)
        {
            printf("Failed to create root signature!\n");
            return false;
        }
    }

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
    GraphicsDesc.pRootSignature = m_RootSignature.Get();
    GraphicsDesc.VS = { VertexShader.Blob()->GetBufferPointer(), VertexShader.Blob()->GetBufferSize() };
    GraphicsDesc.PS = { PixelShader.Blob()->GetBufferPointer(), PixelShader.Blob()->GetBufferSize() };
    GraphicsDesc.DepthStencilState = Utility::MakeDepthStencilDescription();
    GraphicsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    if (m_Device->CreateGraphicsPipelineState(&GraphicsDesc, IID_PPV_ARGS(&m_PipelineState)) != S_OK)
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

    if (m_Device->CreateGraphicsPipelineState(&GraphicsDesc, IID_PPV_ARGS(&m_PipelineStateGUI)) != S_OK)
    {
        Core::Console::WriteLine("Failed to create GUI graphics pipeline state!");
        return false;
    }

    if (m_Device->CreateCommandList(0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_CommandAllocator.Get(),
            m_PipelineState.Get(),
            IID_PPV_ARGS(&m_CommandList)) != S_OK)
    {
        printf("Failed to create command list!\n");
        return false;
    }

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

    m_RenderBuffer.Initialize(m_Device.Get(), 1000, 1000);

    // Vertex Buffer
    {
        const float Offset { 0.25f };
        const float AspectRatio { Window->AspectRatio() };
        Vertices[0] = { { 0.0f, Offset * AspectRatio, 5.0f }, { 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } };
        Vertices[1] = { { -Offset, -Offset * AspectRatio, 5.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } };
        Vertices[2] = { { Offset, -Offset * AspectRatio, 5.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } };

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

        if (m_Device->CreateCommittedResource(&HeapProps,
                D3D12_HEAP_FLAG_NONE,
                &ResourceDesc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &ClearValue,
                IID_PPV_ARGS(&m_DepthStencil)) != S_OK)
        {
            Core::Console::Error("Failed to create depth stencil resource.");
            return false;
        }

        m_Device->CreateDepthStencilView(m_DepthStencil.Get(),
            &DSVDesc,
            m_DSVHeap->GetCPUDescriptorHandleForHeapStart());
    }

    m_RenderBufferGUI.Initialize(m_Device.Get(), 100000, 100000);
    m_RenderBufferGUI.SetStride(sizeof(OctaneGUI::Vertex)).SetFormat(DXGI_FORMAT_R32_UINT);

    // Constant Buffer
    {
        D3D12_HEAP_PROPERTIES HeapProps { Utility::MakeHeapProperties(D3D12_HEAP_TYPE_UPLOAD) };
        D3D12_RESOURCE_DESC ResourceDesc { Utility::MakeResourceDescription() };
        ResourceDesc.Width = sizeof(ConstantBufferData);
        ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        if (m_Device->CreateCommittedResource(&HeapProps,
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

        m_ConstantBufferTableOffset = (MAX_DESCRIPTORS - 1) * m_SRVHeapDescriptorSize;
        D3D12_CPU_DESCRIPTOR_HANDLE CPUDesc { m_SRVHeap->GetCPUDescriptorHandleForHeapStart() };
        CPUDesc.ptr += m_ConstantBufferTableOffset;
        m_Device->CreateConstantBufferView(&ConstantBufferView, CPUDesc);

        D3D12_RANGE Range { 0, 0 };
        if (m_ConstantBuffer->Map(0, &Range, reinterpret_cast<void**>(&m_ConstantBufferAddress)) != S_OK)
        {
            Core::Console::Error("Failed to map constant buffer memory.");
            return false;
        }

        Rectf Bounds { 0.0f, 0.0f, (f32)Window->Size().X, (f32)Window->Size().Y };
        m_ConstantBufferData.Projection =
            Core::Math::PerspectiveMatrixRH(45.0f, Window->AspectRatio(), 0.1f, 100.0f).Transpose();
        m_ConstantBufferData.Orthographic = Core::Math::OrthographicMatrixRH(Bounds, -1.0f, 1.0f).Transpose();
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

bool Renderer::ExecuteCommands()
{
    if (m_CommandList->Close() != S_OK)
    {
        Core::Console::Warning("Failed to close command list.");
        return false;
    }

    ID3D12CommandList* CommandLists[] { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);

    return true;
}

bool Renderer::ResetCommands()
{
    if (m_CommandAllocator->Reset() != S_OK)
    {
        Core::Console::Warning("Failed to reset command allocator!");
        return false;
    }

    if (m_CommandList->Reset(m_CommandAllocator.Get(), m_PipelineState.Get()) != S_OK)
    {
        Core::Console::Warning("Failed to reset command list!");
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

}
}
}

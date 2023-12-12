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

// Need to include before including any Windows headers.
#include "../../External/OctaneGUI/OctaneGUI.h"

#if defined(DEBUG)
    #include "InfoQueue.hpp"
#endif

#include "Renderer.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Containers/Array.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "../../Core/Math/Vertex.hpp"
#include "../../Platform/Window.hpp"
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

    D3D12_RESOURCE_BARRIER Barrier { Utility::MakeResourceBarrierTransition(
        m_RenderTargets[m_FrameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    )};
    m_CommandList->ResourceBarrier(1, &Barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE CPUDesc { m_RTVHeap->GetCPUDescriptorHandleForHeapStart() };
    CPUDesc.ptr = SIZE_T(INT64(CPUDesc.ptr) + INT64(m_FrameIndex) * INT64(m_HeapDescriptorSize));
    m_CommandList->OMSetRenderTargets(1, &CPUDesc, FALSE, nullptr);

    const float ClearColor[] { 0.0f, 0.2f, 0.4f, 1.0f };
    m_CommandList->SetPipelineState(m_PipelineState.Get());
    m_CommandList->ClearRenderTargetView(CPUDesc, ClearColor, 0, nullptr);
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
    if (!Tex.Upload(m_CommandList.Get(), m_SRVHeap.Get(), m_Textures.Size() * m_SRVHeapDescriptorSize, Data, UploadResource))
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

    m_RenderBufferGUI.UploadVertexData(Buffer.GetVertices().data(), static_cast<u64>(Buffer.GetVertexCount()) * sizeof(OctaneGUI::Vertex));
    m_RenderBufferGUI.UploadIndexData(Buffer.GetIndices().data(), static_cast<u64>(Buffer.GetIndexCount()) * sizeof(u32));

    ExecuteCommands();
    WaitForPreviousFrame();

    m_GUICommands
        .Clear()
        .Reserve(Buffer.Commands().size());

    for (const OctaneGUI::DrawCommand& Command : Buffer.Commands())
    {
        m_GUICommands.Push(Command);
    }
}

bool Renderer::LoadPipeline(Platform::Window* Window)
{
    u32 FactoryFlags { 0 };

#if defined(DEBUG)
    Microsoft::WRL::ComPtr<ID3D12Debug> DebugController;
    if (D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController)) == S_OK)
    {
        DebugController->EnableDebugLayer();
        FactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    Microsoft::WRL::ComPtr<IDXGIFactory4> Factory;
    if (CreateDXGIFactory2(FactoryFlags, IID_PPV_ARGS(&Factory)) != S_OK)
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

#if defined(DEBUG)
    InfoQueue::Initialize(m_Device.Get());
#endif

    DXGI_ADAPTER_DESC1 Description;
    Adapter->GetDesc1(&Description);
    SummaryMut().Vendor = Core::Containers::ToString(Description.Description);

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

    D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc { 0 };
    RTVHeapDesc.NumDescriptors = FRAME_COUNT;
    RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (m_Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&m_RTVHeap)) != S_OK)
    {
        printf("Failed to create descriptor heap!\n");
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

        D3D12_DESCRIPTOR_RANGE1 DescRange[1];
        DescRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        DescRange[0].NumDescriptors = 1;
        DescRange[0].BaseShaderRegister = 0;
        DescRange[0].RegisterSpace = 0;
        DescRange[0].Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC;
        DescRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_PARAMETER1 RootParameters[1];
        RootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        RootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        RootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
        RootParameters[0].DescriptorTable.pDescriptorRanges = &DescRange[0];

        D3D12_STATIC_SAMPLER_DESC Sampler = { 0 };
        Sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        Sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        Sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        Sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        Sampler.MipLODBias = 0;
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

        if (m_Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)) != S_OK)
        {
            printf("Failed to create root signature!\n");
            return false;
        }
    }

    const char* Source = R"(
        struct PSInput
        {
            float4 position : SV_POSITION;
            float2 uv : TEXCOORD;
            float4 color : COLOR;
        };

        Texture2D g_Texture : register(t0);
        SamplerState g_Sampler : register(s0);

        PSInput VSMain(float4 position : POSITION, float2 uv : TEXCOORD, float4 color : COLOR)
        {
            PSInput result;

            result.position = position;
            result.uv = uv;
            result.color = color;

            return result;
        }

        float4 PSMain(PSInput input) : SV_TARGET
        {
            return g_Texture.Sample(g_Sampler, input.uv) * input.color;
        }
    )";

    Microsoft::WRL::ComPtr<ID3DBlob> VertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShader;

    u32 CompileFlags { 0 };

#if defined(_DEBUG)
    CompileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Shader ShaderObject;
    bool CompileResult = ShaderObject
        .SetSource(Source)
        .SetName("Default")
        .SetEntryPoint("VSMain")
        .SetTarget("vs_5_0")
        .SetCompileFlags(CompileFlags)
        .AddInputElement({"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .AddInputElement({"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .AddInputElement({"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .Compile();
    if (!CompileResult)
    {
        Core::Console::Warning("Failed to compile vertex shader:\n%s", (LPCSTR)ShaderObject.Errors()->GetBufferPointer());
        return false;
    }
    VertexShader = ShaderObject.Blob();

    CompileResult = ShaderObject
        .SetEntryPoint("PSMain")
        .SetTarget("ps_5_0")
        .Compile();
    if (!CompileResult)
    {
        Core::Console::Warning("Failed to compile pixel shader:\n%s", (LPCSTR)ShaderObject.Errors()->GetBufferPointer());
        return false;
    }
    PixelShader = ShaderObject.Blob();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsDesc { Utility::MakeDefaultGraphicsPipelineState() };
    GraphicsDesc.InputLayout = { ShaderObject.InputElements(), ShaderObject.NumInputElements() };
    GraphicsDesc.pRootSignature = m_RootSignature.Get();
    GraphicsDesc.VS = { VertexShader->GetBufferPointer(), VertexShader->GetBufferSize() };
    GraphicsDesc.PS = { PixelShader->GetBufferPointer(), PixelShader->GetBufferSize() };

    if (m_Device->CreateGraphicsPipelineState(&GraphicsDesc, IID_PPV_ARGS(&m_PipelineState)) != S_OK)
    {
        Core::Console::WriteLine("Failed to create default graphics pipeline state!");
        return false;
    }

    // Begin creation of GUI pipeline state.
    ShaderObject
        .ClearInputElements()
        .AddInputElement({"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .AddInputElement({"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0})
        .AddInputElement({"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0});
    GraphicsDesc.InputLayout = { ShaderObject.InputElements(), ShaderObject.NumInputElements() };

    if (m_Device->CreateGraphicsPipelineState(&GraphicsDesc, IID_PPV_ARGS(&m_PipelineStateGUI)) != S_OK)
    {
        Core::Console::WriteLine("Failed to create GUI graphics pipeline state!");
        return false;
    }

    if (m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), m_PipelineState.Get(), IID_PPV_ARGS(&m_CommandList)) != S_OK)
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
        const float AspectRatio { Window->AspectRatio() };
        Vertices[0] = { {0.0f, 0.25f * AspectRatio, 0.0f}, {0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f} };
        Vertices[1] = { {0.25f, -0.25f * AspectRatio, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} };
        Vertices[2] = { {-0.25f, -0.25f * AspectRatio, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} };

        m_RenderBuffer
            .SetStride(sizeof(Vertex3))
            .UploadVertexData(Vertices, sizeof(Vertices));
    }

    // Index Buffer
    {
        const u32 IndexBufferData[] = { 0, 1, 2 };
        const u32 IndexBufferSize = sizeof(IndexBufferData);

        m_RenderBuffer
            .SetFormat(DXGI_FORMAT_R32_UINT)
            .UploadIndexData(IndexBufferData, IndexBufferSize);
    }

    m_RenderBufferGUI.Initialize(m_Device.Get(), 100000, 100000);
    m_RenderBufferGUI
        .SetStride(sizeof(OctaneGUI::Vertex))
        .SetFormat(DXGI_FORMAT_R32_UINT);

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

            // D3D12CreateDevice will return S_FALSE if ppDevice is null.
            if (D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr) == S_FALSE)
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

            // D3D12CreateDevice will return S_FALSE if ppDevice is null.
            if (D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr) == S_FALSE)
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

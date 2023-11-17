#include <cstdio>
#include "OctaneGUI/OctaneGUI.h"

#ifndef UNICODE
#define UNICODE
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl.h>

#define CLASS_NAME L"LevelSketch"
#define FRAME_COUNT 2

static std::unordered_map<OctaneGUI::Window*, HWND> g_Windows {};

static bool g_InitializedDirectX { false };
static UINT g_FrameIndex { 0 };
static Microsoft::WRL::ComPtr<ID3D12Device> g_Device;
static Microsoft::WRL::ComPtr<ID3D12CommandQueue> g_CommandQueue;
static Microsoft::WRL::ComPtr<ID3D12CommandAllocator> g_CommandAllocator;
static Microsoft::WRL::ComPtr<IDXGISwapChain3> g_SwapChain;
static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> g_Heap;
static Microsoft::WRL::ComPtr<ID3D12Resource> g_RenderTargets[FRAME_COUNT];
static Microsoft::WRL::ComPtr<ID3D12RootSignature> g_RootSignature;
static Microsoft::WRL::ComPtr<ID3D12PipelineState> g_PipelineState;
static Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList1> g_CommandList;
static Microsoft::WRL::ComPtr<ID3D12Resource> g_VertexBuffer;
static Microsoft::WRL::ComPtr<ID3D12Fence> g_Fence;
static D3D12_VERTEX_BUFFER_VIEW g_VertexBufferView { 0 };
static UINT g_HeapDescriptorSize { 0 };
static UINT64 g_FenceValue { 0 };
static HANDLE g_FenceEvent { nullptr };

static IDXGIAdapter1* GetHardwareAdapter(IDXGIFactory1* Factory)
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

static bool LoadPipeline(HWND Window, UINT Width, UINT Height)
{
    Microsoft::WRL::ComPtr<IDXGIFactory4> Factory;
    if (CreateDXGIFactory2(0, IID_PPV_ARGS(&Factory)) != S_OK)
    {
        printf("Failed in CreateDXGIFactor2!\n");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter { GetHardwareAdapter(Factory.Get()) };
    if (D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&g_Device)) != S_OK)
    {
        printf("Failed in D3D12CreateDevice!\n");
        return false;
    }

    D3D12_COMMAND_QUEUE_DESC CommandQueueDescription { 0 };
    CommandQueueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    CommandQueueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    if (g_Device->CreateCommandQueue(&CommandQueueDescription, IID_PPV_ARGS(&g_CommandQueue)) != S_OK)
    {
        printf("Failed to create device command queue!\n");
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 SwapChainDescription { 0 };
    SwapChainDescription.BufferCount = FRAME_COUNT;
    SwapChainDescription.Width = Width;
    SwapChainDescription.Height = Height;
    SwapChainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    SwapChainDescription.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain;
    if (Factory->CreateSwapChainForHwnd(
        g_CommandQueue.Get(),
        Window,
        &SwapChainDescription,
        nullptr,
        nullptr,
        &SwapChain
    ) != S_OK)
    {
        printf("Failed to create swap chain!\n");
        return false;
    }

    if (Factory->MakeWindowAssociation(Window, DXGI_MWA_NO_ALT_ENTER) != S_OK)
    {
        printf("Failed to make window association!\n");
        return false;
    }

    if (SwapChain.As(&g_SwapChain) != S_OK)
    {
        printf("Failed to retrieve swap chain!\n");
        return false;
    }

    g_FrameIndex = g_SwapChain->GetCurrentBackBufferIndex();

    D3D12_DESCRIPTOR_HEAP_DESC HeapDescription { 0 };
    HeapDescription.NumDescriptors = FRAME_COUNT;
    HeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    HeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (g_Device->CreateDescriptorHeap(&HeapDescription, IID_PPV_ARGS(&g_Heap)) != S_OK)
    {
        printf("Failed to create descriptor heap!\n");
        return false;
    }

    g_HeapDescriptorSize = g_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle { g_Heap->GetCPUDescriptorHandleForHeapStart() };
    for (UINT I = 0; I < FRAME_COUNT; ++I)
    {
        if (g_SwapChain->GetBuffer(I, IID_PPV_ARGS(&g_RenderTargets[I])) != S_OK)
        {
            printf("Failed to get buffer %d for swap chain!\n", I);
            return false;
        }

        g_Device->CreateRenderTargetView(g_RenderTargets[I].Get(), nullptr, CPUHandle);
        CPUHandle.ptr = SIZE_T(INT64(CPUHandle.ptr) + INT64(1) * INT64(g_HeapDescriptorSize));
    }

    if (g_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_CommandAllocator)) != S_OK)
    {
        printf("Failed to create command allocator!\n");
        return false;
    }

    return true;
}

static bool LoadAssets(float AspectRatio)
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

    if (g_Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&g_RootSignature)) != S_OK)
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
    GraphicsDesc.pRootSignature = g_RootSignature.Get();
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

    if (g_Device->CreateGraphicsPipelineState(&GraphicsDesc, IID_PPV_ARGS(&g_PipelineState)) != S_OK)
    {
        printf("Failed to create graphics pipeline state!\n");
        return false;
    }

    if (g_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_CommandAllocator.Get(), g_PipelineState.Get(), IID_PPV_ARGS(&g_CommandList)) != S_OK)
    {
        printf("Failed to create command list!\n");
        return false;
    }

    if (g_CommandList->Close() != S_OK)
    {
        printf("Failed to close command list!\n");
        return false;
    }

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
    if (g_Device->CreateCommittedResource(
        &HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &ResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&g_VertexBuffer)
    ) != S_OK)
    {
        printf("Failed to create committed resource!\n");
        return false;
    }

    UINT8* VertexData { nullptr };
    D3D12_RANGE Range { 0, 0 };
    if (g_VertexBuffer->Map(0, &Range, reinterpret_cast<void**>(&VertexData)) != S_OK)
    {
        printf("Failed to map vertex buffer!\n");
        return false;
    }
    memcpy(VertexData, Vertices, sizeof(Vertices));
    g_VertexBuffer->Unmap(0, nullptr);

    g_VertexBufferView.BufferLocation = g_VertexBuffer->GetGPUVirtualAddress();
    g_VertexBufferView.StrideInBytes = sizeof(float) * 7; // 3 floats for position, 4 floats for color.
    g_VertexBufferView.SizeInBytes = VerticesSize;

    if (g_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_Fence)) != S_OK)
    {
        printf("Failed to create fence!\n");
        return false;
    }
    g_FenceValue = 1;

    g_FenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (g_FenceEvent == nullptr)
    {
        printf("Failed to create fence event!\n");
        return false;
    }

    return true;
}

static bool InitializeDirectX(HWND Window, UINT Width, UINT Height)
{
    if (g_InitializedDirectX)
    {
        return g_InitializedDirectX;
    }

    if (!LoadPipeline(Window, Width, Height))
    {
        return false;
    }

    const float AspectRatio { (float)Width / (float)Height };
    if (!LoadAssets(AspectRatio))
    {
        return false;
    }

    printf("Initialized DirectX!\n");
    g_InitializedDirectX = true;
    return true;
}

LRESULT WinProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
    case WM_CLOSE:
        for (std::unordered_map<OctaneGUI::Window*, HWND>::const_iterator It { g_Windows.begin() }; It != g_Windows.end(); ++It)
        {
            if (It->second == hWnd)
            {
                g_Windows.erase(It);
                break;
            }
        }

        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default: break;
    }
    return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

void OnWindowAction(OctaneGUI::Window* Window, OctaneGUI::WindowAction Action)
{
    switch (Action)
    {
    case OctaneGUI::WindowAction::Create:
    {
        HWND Handle { CreateWindowExW(
            WS_EX_CLIENTEDGE,
            CLASS_NAME,
            OctaneGUI::String::ToWide(Window->GetTitle()).c_str(),
            WS_OVERLAPPEDWINDOW,
            (int)Window->GetPosition().X,
            (int)Window->GetPosition().Y,
            (int)Window->GetSize().X,
            (int)Window->GetSize().Y,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        ) };

        if (Handle != INVALID_HANDLE_VALUE)
        {
            g_Windows[Window] = Handle;
            ShowWindow(Handle, SW_NORMAL);

            InitializeDirectX(Handle, (UINT)Window->GetSize().X, (UINT)Window->GetSize().Y);
        }
        else
        {
            printf("Failed to create window!\n");
        }
    } break;

    case OctaneGUI::WindowAction::Destroy:
    {
        if (g_Windows.find(Window) != g_Windows.end())
        {
            if (!DestroyWindow(g_Windows[Window]))
            {
                printf("Failed to destroy window!\n");
            }

            g_Windows.erase(Window);
        }
    } break;

    case OctaneGUI::WindowAction::Raise:
    {
        if (g_Windows.find(Window) != g_Windows.end())
        {
            SetForegroundWindow(g_Windows[Window]);
        }
    } break;

    case OctaneGUI::WindowAction::Position:
    {
        if (g_Windows.find(Window) != g_Windows.end())
        {
            SetWindowPos(
                g_Windows[Window],
                HWND_TOP,
                (int)Window->GetPosition().X,
                (int)Window->GetPosition().Y,
                0,
                0,
                SWP_NOSIZE
            );
        }
    } break;

    case OctaneGUI::WindowAction::Enable:
    case OctaneGUI::WindowAction::Disable:
    case OctaneGUI::WindowAction::Size:
    case OctaneGUI::WindowAction::Minimize:
    case OctaneGUI::WindowAction::Maximize:
    default: break;
    }
}

OctaneGUI::Event OnEvent(OctaneGUI::Window* Window)
{
    if (g_Windows.find(Window) == g_Windows.end())
    {
        return { OctaneGUI::Event::Type::WindowClosed };
    }

    HWND Handle { g_Windows[Window] };

    MSG Msg {};
    while (PeekMessageW(&Msg, Handle, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Msg);
        DispatchMessageW(&Msg);
    }

    return { OctaneGUI::Event::Type::None };
}

int main(int argc, char** argv)
{
    const char* Stream = R"({
        "Windows": {
            "Main": {"Title": "Level Sketch", "Width": 1280, "Height": 720,
                "MenuBar": {},
                "Body": {"Controls": []}
            }
        }
    })";

    WNDCLASSEXW Class;
    ZeroMemory(&Class, sizeof(WNDCLASSEXW));
    Class.cbSize = sizeof(Class);
    Class.style = CS_HREDRAW | CS_VREDRAW;
    Class.lpszClassName = CLASS_NAME;
    Class.lpfnWndProc = WinProc;
    Class.hInstance = GetModuleHandleW(nullptr);

    if (RegisterClassExW(&Class) == 0)
    {
        printf("Failed to register windows class.\n");
        return -1;
    }

    OctaneGUI::Application Application;
    Application
        .SetOnWindowAction(OnWindowAction)
        .SetOnEvent(OnEvent);

    std::unordered_map<std::string, OctaneGUI::ControlList> List;
    bool Success = Application
        .SetCommandLine(argc, argv)
        .Initialize(Stream, List);
    
    if (!Success)
    {
        printf("Failed to initialize application!\n");
        return 0;
    }

    int Return { Application.Run() };

    CloseHandle(g_FenceEvent);
    UnregisterClassW(CLASS_NAME, nullptr);

    return Return;
}

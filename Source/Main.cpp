#include <cstdio>
#include "OctaneGUI/OctaneGUI.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
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
static UINT g_HeapDescriptorSize { 0 };

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

    UnregisterClassW(CLASS_NAME, nullptr);

    return Return;
}

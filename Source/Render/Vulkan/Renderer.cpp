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

#include "Renderer.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Version.hpp"
#include "../../Core/Math/Vertex.hpp"
#include "../../Platform/FileSystem.hpp"
#include "../../Platform/Window.hpp"
#include "Errors.hpp"
#include "Loader.hpp"
#include "Shader.hpp"

#if defined(PLATFORM_SDL2)
    #include "SDL2/SDL.h"
    #include "SDL2/SDL_syswm.h"
#endif

#if defined(DEBUG)
    #include "DebugUtils.hpp"
#endif

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

// FIXME: Better orginazation can be used here.
#if defined(DEBUG)
    static const Array<const char*> ValidationLayers {
        "VK_LAYER_KHRONOS_validation"
    };
#endif

Renderer::Renderer()
    : LevelSketch::Render::Renderer()
{
}

bool Renderer::Initialize()
{
    if (!Loader::Instance().Initialize())
    {
        return false;
    }

    const u32 Version { VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION) };
    VkApplicationInfo AppInfo {};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = APP_NAME;
    AppInfo.applicationVersion = Version;
    AppInfo.pEngineName = APP_NAME;
    AppInfo.engineVersion = Version;
    AppInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;
    CreateInfo.enabledLayerCount = 0;

    Array<VkExtensionProperties> ExtProperties;
    if (!Loader::Instance().GetInstanceExtensionProperties(ExtProperties))
    {
        return false;
    }

    Array<const char*> ExtPtrs;
    if (!GetRequiredExtensionProperties(ExtProperties, ExtPtrs))
    {
        return false;
    }

#if defined(DEBUG)
    Array<const char*> LayerPtrs;
    if (GetExistingLayers(ValidationLayers, LayerPtrs))
    {
        Core::Console::WriteLine("Enabling validation layers");
        CreateInfo.enabledLayerCount = static_cast<u32>(LayerPtrs.Size());
        CreateInfo.ppEnabledLayerNames = LayerPtrs.Data();

        ExtPtrs.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkDebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo {
        DebugUtils::CreateInfo()
    };
    CreateInfo.pNext = &DebugUtilsMessengerCreateInfo;
#endif

    CreateInfo.enabledExtensionCount = static_cast<u32>(ExtPtrs.Size());
    CreateInfo.ppEnabledExtensionNames = ExtPtrs.Data();

    VkResult Result { vkCreateInstance(&CreateInfo, nullptr, &m_Instance) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create instance. Error: %s", Errors::ToString(Result));
        return false;
    }

#if defined(DEBUG)
    DebugUtils::Instance().Initialize(m_Instance);
#endif

    return true;
}

bool Renderer::Initialize(Platform::Window* Window)
{
    if (!m_Surface.IsValid())
    {
        Vector2i PixelRes {};
#if defined(PLATFORM_SDL2)
        SDL_Window* Handle { reinterpret_cast<SDL_Window*>(Window->Handle()) };
        SDL_SysWMinfo Info {};
        SDL_GetVersion(&Info.version);
        if (SDL_GetWindowWMInfo(Handle, &Info) != SDL_TRUE)
        {
            Core::Console::Error("Failed to retrieve native window information. Error: %s", SDL_GetError());
            return false;
        }

        if (!m_Surface.Initialize(m_Instance, Info.info.x11.window, Info.info.x11.display))
        {
            return false;
        }

        SDL_GetWindowSizeInPixels(Handle, &PixelRes.X, &PixelRes.Y);
#else
    #error "Renderer::Initialize(Platform::Window*) needs platform specific implementation!"
#endif

        Array<const char*> LayerPtrs;
#if defined(DEBUG)
        GetExistingLayers(ValidationLayers, LayerPtrs);
#endif

        if (!m_Device.Initialize(m_Instance, m_Surface, LayerPtrs))
        {
            return false;
        }

        if (!m_CommandPool.Initialize(m_Device))
        {
            return false;
        }

        if (!m_CommandPool.InitializeBuffers(m_Device, FRAMES_IN_FLIGHT))
        {
            return false;
        }

        VkExtent2D Extents { static_cast<u32>(PixelRes.X), static_cast<u32>(PixelRes.Y) };
        if (!m_SwapChain.Initialize(m_Device, m_Surface, Extents))
        {
            Core::Console::Error("Failed to initialize swap chain.");
            return false;
        }

        const String ShaderPath { Platform::FileSystem::CombinePaths(
            Platform::FileSystem::ApplicationPath(),
            "Content/Shaders/GLSL")
        };

        Shader Vertex {};
        if (!Vertex.Load(m_Device, Platform::FileSystem::CombinePaths(ShaderPath, "Test.vert").Data()))
        {
            return false;
        }

        VkVertexInputBindingDescription BindingDesc {};
        BindingDesc.binding = 0;
        BindingDesc.stride = sizeof(Vertex3);
        BindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription AttributeDesc[3] = {};
        AttributeDesc[0].binding = 0;
        AttributeDesc[0].location = 0;
        AttributeDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        AttributeDesc[0].offset = offsetof(Vertex3, Position);

        AttributeDesc[1].binding = 0;
        AttributeDesc[1].location = 1;
        AttributeDesc[1].format = VK_FORMAT_R32G32_SFLOAT;
        AttributeDesc[1].offset = offsetof(Vertex3, UV);

        AttributeDesc[2].binding = 0;
        AttributeDesc[2].location = 2;
        AttributeDesc[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        AttributeDesc[2].offset = offsetof(Vertex3, Color);

        Vertex
            .PushBinding(BindingDesc)
            .PushAttribute(AttributeDesc[0])
            .PushAttribute(AttributeDesc[1])
            .PushAttribute(AttributeDesc[2]);

        Shader Fragment {};
        if (!Fragment.Load(m_Device, Platform::FileSystem::CombinePaths(ShaderPath, "Test.frag").Data()))
        {
            return false;
        }

        if (!m_Pipeline.Initialize(m_Device, m_SwapChain, Vertex, Fragment))
        {
            return false;
        }

        if (!m_SwapChain.InitializeFramebuffers(m_Device, m_Pipeline))
        {
            return false;
        }

        for (Sync& Sync_ : m_Syncs)
        {
            if (!Sync_.Initialize(m_Device))
            {
                return false;
            }
        }

        Vertex.Shutdown(m_Device);
        Fragment.Shutdown(m_Device);

        const Vertex3 Vertices[4] {
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
            {{0.5f, 0.5f, 0.0f,}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f,}, {0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}
        };
        const u64 VerticesSize { sizeof(Vertex3) * ARRAY_COUNT(Vertices) };

        const u32 Indices[] { 0, 2, 1, 0, 3, 2 };
        const u64 IndicesSize { sizeof(u32) * ARRAY_COUNT(Indices) };

        if (!m_RenderBuffer.Initialize(m_Device, VerticesSize, IndicesSize))
        {
            return false;
        }

        m_RenderBuffer.VertexBuffer().Upload(m_Device, m_CommandPool, Vertices, VerticesSize);
        m_RenderBuffer.IndexBuffer().Upload(m_Device, m_CommandPool, Indices, IndicesSize);

        Core::Console::WriteLine("Initialized Vulkan");
    }

    return true;
}

void Renderer::Shutdown()
{
    m_Device.WaitForIdle();

#if defined(DEBUG)
    DebugUtils::Instance().Shutdown(m_Instance);
#endif

    m_RenderBuffer.Shutdown(m_Device);

    for (Sync& Sync_ : m_Syncs)
    {
        Sync_.Shutdown(m_Device);
    }

    m_Pipeline.Shutdown(m_Device);
    m_SwapChain.Shutdown(m_Device);
    m_CommandPool.Shutdown(m_Device);
    m_Device.Shutdown();
    m_Surface.Shutdown(m_Instance);

    if (m_Instance != nullptr)
    {
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }

    Loader::Instance().Shutdown();
}

void Renderer::Render(Platform::Window*)
{
    const Sync& CurrentSync { m_Syncs[m_FrameIndex] };

    CurrentSync.WaitForFence(m_Device);
    const u32 SwapChainFrameIndex { CurrentSync.FrameIndex(m_Device, m_SwapChain) };

    const CommandBuffer& CmdBuffer { m_CommandPool.Buffer(m_FrameIndex) };

    CmdBuffer.Reset();
    CmdBuffer.BeginRecord(m_Pipeline, m_SwapChain, SwapChainFrameIndex);
    CmdBuffer.BindBuffers(m_RenderBuffer);
    CmdBuffer.DrawVerticesIndexed(6, 1, 0, 0, 0);
    CmdBuffer.EndRecord();
    CmdBuffer.Submit(m_Device, CurrentSync);

    m_SwapChain.Present(m_Device, CurrentSync, SwapChainFrameIndex);

    m_FrameIndex = (m_FrameIndex + 1) % FRAMES_IN_FLIGHT;
}

u32 Renderer::LoadTexture(const void*, u32, u32, u8)
{
    return 1;
}

void Renderer::UploadGUIData(OctaneGUI::Window*, const OctaneGUI::VertexBuffer&)
{
}

bool Renderer::GetRequiredExtensionProperties(const Array<VkExtensionProperties>& Properties, Array<const char*>& Ptrs) const
{
    bool FoundSurface { false };

    for (const VkExtensionProperties& Property : Properties)
    {
        const String Name { Property.extensionName };
        if (Name == "VK_KHR_surface"
            || Name == "VK_KHR_xlib_surface"
            || Name == "VK_KHR_xcb_surface"
            || Name == "VK_KHR_wayland_surface")
        {
            FoundSurface |= Name == "VK_KHR_surface";
            Ptrs.Push(Property.extensionName);
        }
    }

    if (!FoundSurface)
    {
        Core::Console::Error("Failed to load window surface extensions.");
        return false;
    }

    return true;
}

bool Renderer::GetExistingLayers(const Array<const char*> Layers, Array<const char*>& Ptrs) const
{
    Ptrs.Clear();

    u32 Count { 0 };
    VkResult Result { vkEnumerateInstanceLayerProperties(&Count, nullptr) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::WriteLine("Failed to retrieve layer properties count. Error: %s", Errors::ToString(Result));
        return false;
    }

    Array<VkLayerProperties> Properties;
    Properties.Resize(Count);
    Result = vkEnumerateInstanceLayerProperties(&Count, Properties.Data());
    if (Result != VK_SUCCESS)
    {
        Core::Console::WriteLine("Failed to retrieve layer properties. Error: %s", Errors::ToString(Result));
        return false;
    }

    for (const char* Name : Layers)
    {
        for (const VkLayerProperties& Layer : Properties)
        {
            if (String(Name) == Layer.layerName)
            {
                Ptrs.Push(Name);
            }
        }
    }

    return !Ptrs.IsEmpty();
}

}
}
}

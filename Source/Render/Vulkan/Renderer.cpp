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
#include "../../Platform/Window.hpp"
#include "Errors.hpp"
#include "Loader.hpp"

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
#else
    #error "Renderer::Initialize(Platform::Window*) needs platform specific implementation!"
#endif

        if (!GetPhysicalDevice())
        {
            return false;
        }

        Array<const char*> LayerPtrs;
#if defined(DEBUG)
        GetExistingLayers(ValidationLayers, LayerPtrs);
#endif

        if (!m_LogicalDevice.Initialize(m_PhysicalDevice, LayerPtrs))
        {
            Core::Console::Error("Failed to initialize logical device.");
            return false;
        }

        if (!m_GraphicsQueue.Initialize(m_LogicalDevice, m_PhysicalDevice.QueueFamilyIndex().Graphics()))
        {
            Core::Console::Error("Failed to initialize graphics queue.");
            return false;
        }

        if (!m_PresentQueue.Initialize(m_LogicalDevice, m_PhysicalDevice.QueueFamilyIndex().Present()))
        {
            Core::Console::Error("Failed to initialize present queue.");
            return false;
        }
    }

    return true;
}

void Renderer::Shutdown()
{
#if defined(DEBUG)
    DebugUtils::Instance().Shutdown(m_Instance);
#endif

    m_Surface.Shutdown(m_Instance);
    m_LogicalDevice.Shutdown();

    if (m_Instance != nullptr)
    {
        vkDestroyInstance(m_Instance, nullptr);
    }

    Loader::Instance().Shutdown();
}

void Renderer::Render(Platform::Window*)
{
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

bool Renderer::GetPhysicalDevice()
{
    Array<PhysicalDevice> Devices { PhysicalDevice::GetDevices(m_Instance, m_Surface) };

    if (Devices.IsEmpty())
    {
        Core::Console::Error("Failed to find a physical device.");
        return false;
    }

    Core::Console::WriteLine("Found %lu devices", Devices.Size());
    for (const PhysicalDevice& Device : Devices)
    {
        if (Device.QueueFamilyIndex().IsComplete())
        {
            m_PhysicalDevice = Device;
            break;
        }
    }

    if (m_PhysicalDevice.QueueFamilyIndex().IsComplete())
    {
        Core::Console::WriteLine("Selected Device:");
        m_PhysicalDevice.PrintInfo();
    }
    else
    {
        Core::Console::Error("Failed to find a device that supports graphics queues.");
    }

    return m_PhysicalDevice.QueueFamilyIndex().IsComplete();
}

}
}
}

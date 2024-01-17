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

#include "Loader.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Containers/Array.hpp"
#include "Errors.hpp"

#include <dlfcn.h>

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

Loader& Loader::Instance()
{
    static Loader Instance;
    return Instance;
}

bool Loader::Initialize()
{
    if (m_Handle == nullptr)
    {
        m_Handle = dlopen("libvulkan.so.1", RTLD_LAZY | RTLD_LOCAL);
        if (m_Handle == nullptr)
        {
            Core::Console::Error("Failed to load vulkan library.\nError: %s", dlerror());
            return false;
        }
    }

    if (!LoadGetInstanceProcAddr())
    {
        return false;
    }

    return IsInitialized();
}

bool Loader::IsInitialized() const
{
    return m_Handle != nullptr;
}

void Loader::Shutdown()
{
    if (m_Handle != nullptr)
    {
        dlclose(m_Handle);
    }
}

bool Loader::GetInstanceExtensionProperties(Array<VkExtensionProperties>& Properties)
{
    Properties.Clear();

    if (!LoadGetInstanceProcAddr())
    {
        return false;
    }

    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionPropertiesFn {
        reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(m_GetInstanceProcAddr(
            nullptr, "vkEnumerateInstanceExtensionProperties"))
    };

    if (vkEnumerateInstanceExtensionPropertiesFn == nullptr)
    {
        Core::Console::Error("Failed to load vkEnumerateInstanceExtensionProperties function!");
        return false;
    }

    u32 PropertyCount { 0 };
    VkResult Result { vkEnumerateInstanceExtensionPropertiesFn(nullptr, &PropertyCount, nullptr) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve extension properties count. Error: %s", Errors::ToString(Result));
        return false;
    }

    Properties.Resize(PropertyCount);
    Result = vkEnumerateInstanceExtensionPropertiesFn(nullptr, &PropertyCount, Properties.Data());
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve extenstion properties. Error: %s", Errors::ToString(Result));
        return false;
    }

    return true;
}

Loader::Loader()
{
}

bool Loader::LoadGetInstanceProcAddr()
{
    if (m_GetInstanceProcAddr != nullptr)
    {
        return true;
    }

    if (!IsInitialized())
    {
        Core::Console::Error("Unable to load GetInstanceProcAddr. Vulkan Loader has not been initialized!");
        return false;
    }

    if (m_GetInstanceProcAddr == nullptr)
    {
        m_GetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(m_Handle, "vkGetInstanceProcAddr"));
        if (m_GetInstanceProcAddr == nullptr)
        {
            Core::Console::Error("Failed to load vkGetInstanceProcAddr from Vulkan library.\nError: %s", dlerror());
            return false;
        }
    }

    return true;
}

}
}
}

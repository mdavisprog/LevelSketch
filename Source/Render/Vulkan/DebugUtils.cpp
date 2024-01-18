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

#include "DebugUtils.hpp"
#include "../../Core/Console.hpp"
#include "Errors.hpp"
#include "Loader.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

static VKAPI_ATTR VkBool32 VKAPI_CALL MessageCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT /*Severity*/,
    VkDebugUtilsMessageTypeFlagsEXT /*Type*/,
    const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
    void* /*UserData*/
)
{
    Core::Console::WriteLine(CallbackData->pMessage);
    return VK_FALSE;
}

DebugUtils& DebugUtils::Instance()
{
    static DebugUtils Instance;
    return Instance;
}

VkDebugUtilsMessengerCreateInfoEXT DebugUtils::CreateInfo()
{
    VkDebugUtilsMessengerCreateInfoEXT Result {};
    Result.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    Result.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    Result.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    Result.pfnUserCallback = MessageCallback;

    return Result;
}

bool DebugUtils::Initialize(VkInstance Instance)
{
    if (m_Messenger != nullptr)
    {
        return true;
    }

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXTFn {
        Loader::Instance().LoadFn<PFN_vkCreateDebugUtilsMessengerEXT>(Instance, "vkCreateDebugUtilsMessengerEXT")
    };

    if (vkCreateDebugUtilsMessengerEXTFn == nullptr)
    {
        Core::Console::WriteLine("Failed to load vkCreateDebugUtilsMessengerEXT function.");
        return false;
    }

    VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo { CreateInfo() };
    VkResult Result { vkCreateDebugUtilsMessengerEXTFn(Instance, &MessengerCreateInfo, nullptr, &m_Messenger) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::WriteLine("Failed to create debug utils messenger. Error: %s", Errors::ToString(Result));
        return false;
    }

    return m_Messenger != nullptr;
}

void DebugUtils::Shutdown(VkInstance Instance)
{
    if (m_Messenger == nullptr)
    {
        return;
    }

    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXTFn {
        Loader::Instance().LoadFn<PFN_vkDestroyDebugUtilsMessengerEXT>(Instance, "vkDestroyDebugUtilsMessengerEXT")
    };

    if (vkDestroyDebugUtilsMessengerEXTFn == nullptr)
    {
        Core::Console::WriteLine("Failed to load vkDestroyDebugUtilsMessengerEXT function.");
        return;
    }

    vkDestroyDebugUtilsMessengerEXTFn(Instance, m_Messenger, nullptr);
    m_Messenger = VK_NULL_HANDLE;
}

DebugUtils::DebugUtils()
{
}

}
}
}

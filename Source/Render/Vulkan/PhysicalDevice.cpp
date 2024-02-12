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

#include "PhysicalDevice.hpp"
#include "../../Core/Console.hpp"
#include "Errors.hpp"
#include "Surface.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

const Array<const char*> PhysicalDevice::s_RequiredExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

static PhysicalDevice::Info GatherInfo(VkPhysicalDevice Device)
{
    PhysicalDevice::Info Result {};

    VkPhysicalDeviceProperties Properties;
    vkGetPhysicalDeviceProperties(Device, &Properties);

    Result.APIVersion_Major = VK_API_VERSION_MAJOR(Properties.apiVersion);
    Result.APIVersion_Minor = VK_API_VERSION_MINOR(Properties.apiVersion);
    Result.APIVersion_Patch = VK_API_VERSION_PATCH(Properties.apiVersion);
    Result.DriverVersion = Properties.driverVersion;
    Result.VendorID = Properties.vendorID;
    Result.DeviceID = Properties.deviceID;
    Result.Name = Properties.deviceName;

    return Result;
}

static PhysicalDevice::SupportDetails QuerySupportDetails(VkPhysicalDevice Device, Surface const* Surface_)
{
    PhysicalDevice::SupportDetails Details {};

    VkResult Result {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface_->Get(), &Details.SurfaceCapabilities)
    };

    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve surface capabilities. Error: %s", Errors::ToString(Result));
        return Details;
    }

    // Formats
    {
        u32 Count { 0 };
        Result = vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface_->Get(), &Count, nullptr);

        if (Result != VK_SUCCESS)
        {
            Core::Console::Error("Failed to retrieve surface formats count. Error: %s", Errors::ToString(Result));
            return Details;
        }

        Details.Formats.Resize(Count);
        Result = vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface_->Get(), &Count, Details.Formats.Data());

        if (Result != VK_SUCCESS)
        {
            Core::Console::Error("Failed to retrieve surface formats. Error: %s", Errors::ToString(Result));
            return Details;
        }
    }

    // Present Modes
    {
        u32 Count { 0 };
        Result = vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface_->Get(), &Count, nullptr);

        if (Result != VK_SUCCESS)
        {
            Core::Console::Error("Failed to retrieve surface present modes count. Error: %s", Errors::ToString(Result));
            return Details;
        }

        Details.PresentModes.Resize(Count);
        Result =
            vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface_->Get(), &Count, Details.PresentModes.Data());

        if (Result != VK_SUCCESS)
        {
            Core::Console::Error("Failed to retrieve surface present modes. Error: %s", Errors::ToString(Result));
            return Details;
        }
    }

    return Details;
}

static bool AreRequiredExtensionsSupported(VkPhysicalDevice Device, const Array<const char*>& RequiredExtensions)
{
    u32 Count { 0 };
    VkResult Result { vkEnumerateDeviceExtensionProperties(Device, nullptr, &Count, nullptr) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve device extension count. Error: %s", Errors::ToString(Result));
        return false;
    }

    Array<VkExtensionProperties> Extensions;
    Extensions.Resize(Count);

    Result = vkEnumerateDeviceExtensionProperties(Device, nullptr, &Count, Extensions.Data());
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve device extensions. Error: %s", Errors::ToString(Result));
        return false;
    }

    Array<String> Required;
    for (const char* Name : RequiredExtensions)
    {
        Required.Push(Name);
    }

    for (VkExtensionProperties& Extension : Extensions)
    {
        Required.Remove(Extension.extensionName);
    }

    return Required.IsEmpty();
}

static QueueFamily QueryQueueFamily(VkPhysicalDevice Device, Surface const* Surface_)
{
    u32 Count { 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &Count, nullptr);

    Array<VkQueueFamilyProperties> Properties;
    Properties.Resize(Count);
    vkGetPhysicalDeviceQueueFamilyProperties(Device, &Count, Properties.Data());

    QueueFamily QF {};

    u32 Index { 0 };
    for (const VkQueueFamilyProperties& Property : Properties)
    {
        if (Property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            QF.Graphics = Index;
        }

        VkBool32 Present { VK_FALSE };
        VkResult Result { vkGetPhysicalDeviceSurfaceSupportKHR(Device, Index, Surface_->Get(), &Present) };
        if (Result == VK_SUCCESS && Present)
        {
            QF.Present = Index;
        }

        Index++;
    }

    return QF;
}

UniquePtr<PhysicalDevice> PhysicalDevice::BestDevice(VkInstance Instance, Surface const* Surface_)
{
    u32 Count { 0 };
    VkResult Result { vkEnumeratePhysicalDevices(Instance, &Count, nullptr) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve physical device count. Error: %s", Errors::ToString(Result));
        return nullptr;
    }

    if (Count == 0)
    {
        Core::Console::Error("No physical devices found.");
        return nullptr;
    }

    Array<VkPhysicalDevice> PhysicalDevices;
    PhysicalDevices.Resize(Count);
    Result = vkEnumeratePhysicalDevices(Instance, &Count, PhysicalDevices.Data());
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve physical devices. Error: %s", Errors::ToString(Result));
        return nullptr;
    }

    struct PendingDevice
    {
        VkPhysicalDevice Device { VK_NULL_HANDLE };
        QueueFamily QF {};
        bool ExtensionsSupported { false };
        SupportDetails Details {};
    };

    Array<PendingDevice> PendingDevices {};
    for (VkPhysicalDevice Device : PhysicalDevices)
    {
        PendingDevice Pending {};
        Pending.Device = Device;
        Pending.QF = QueryQueueFamily(Device, Surface_);
        Pending.ExtensionsSupported = AreRequiredExtensionsSupported(Device, s_RequiredExtensions);
        Pending.Details = QuerySupportDetails(Device, Surface_);
        PendingDevices.Push(Pending);
    }

    UniquePtr<PhysicalDevice> Best { nullptr };

    for (const PendingDevice& Pending : PendingDevices)
    {
        if (!(Pending.QF.Graphics.HasValue() && Pending.QF.Present.HasValue()))
        {
            continue;
        }

        if (!Pending.ExtensionsSupported)
        {
            continue;
        }

        if (Pending.Details.Formats.IsEmpty() || Pending.Details.PresentModes.IsEmpty())
        {
            continue;
        }

        Best = UniquePtr<PhysicalDevice>::New();
        Best->m_Device = Pending.Device;
        Best->m_QueueFamily = Pending.QF;
        Best->m_SupportDetails = Pending.Details;
        Best->m_Info = GatherInfo(Pending.Device);

        break;
    }

    return Best;
}

PhysicalDevice::PhysicalDevice()
{
}

VkPhysicalDevice PhysicalDevice::Get() const
{
    return m_Device;
}

const QueueFamily& PhysicalDevice::GetQueueFamily() const
{
    return m_QueueFamily;
}

const PhysicalDevice::SupportDetails& PhysicalDevice::GetSupportDetails() const
{
    return m_SupportDetails;
}

const PhysicalDevice::Info& PhysicalDevice::GetInfo() const
{
    return m_Info;
}

}
}
}

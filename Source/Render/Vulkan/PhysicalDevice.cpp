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
#include "../../Core/Containers/Array.hpp"
#include "Errors.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

Array<PhysicalDevice> PhysicalDevice::GetDevices(VkInstance Instance)
{
    Array<PhysicalDevice> Devices;

    u32 Count { 0 };
    VkResult Result { vkEnumeratePhysicalDevices(Instance, &Count, nullptr) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve physical device count. Error: %s", Errors::ToString(Result));
        return Devices;
    }

    if (Count == 0)
    {
        Core::Console::Error("No physical devices found.");
        return Devices;
    }

    Array<VkPhysicalDevice> PhysicalDevices;
    PhysicalDevices.Resize(Count);
    Result = vkEnumeratePhysicalDevices(Instance, &Count, PhysicalDevices.Data());
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to retrieve physical devices. Error: %s", Errors::ToString(Result));
        return Devices;
    }

    for (const VkPhysicalDevice& Device : PhysicalDevices)
    {
        PhysicalDevice NewDevice { Device };
        NewDevice
            .GatherInfo()
            .FindQueueFamily();
        Devices.Push(NewDevice);
    }

    return Devices;
}

PhysicalDevice::PhysicalDevice()
{
}

void PhysicalDevice::PrintInfo() const
{
    Core::Console::WriteLine("Device: %s", m_Info.Name.Data());
    Core::Console::WriteLine("API Version: %d.%d.%d",
        m_Info.APIVersion_Major, m_Info.APIVersion_Minor, m_Info.APIVersion_Patch);
    Core::Console::WriteLine("Driver Version: %d", m_Info.DriverVersion);
    Core::Console::WriteLine("Vendor ID: %d", m_Info.VendorID);
    Core::Console::WriteLine("Device ID: %d", m_Info.DeviceID);
}

bool PhysicalDevice::SupportsGraphics() const
{
    return m_QueueFamily.Graphics.HasValue();
}

PhysicalDevice::PhysicalDevice(VkPhysicalDevice Device)
    : m_Device(Device)
{
}

PhysicalDevice& PhysicalDevice::GatherInfo()
{
    if (m_Device == VK_NULL_HANDLE)
    {
        return *this;
    }

    VkPhysicalDeviceProperties Properties;
    vkGetPhysicalDeviceProperties(m_Device, &Properties);

    m_Info.APIVersion_Major = VK_API_VERSION_MAJOR(Properties.apiVersion);
    m_Info.APIVersion_Minor = VK_API_VERSION_MINOR(Properties.apiVersion);
    m_Info.APIVersion_Patch = VK_API_VERSION_PATCH(Properties.apiVersion);
    m_Info.DriverVersion = Properties.driverVersion;
    m_Info.VendorID = Properties.vendorID;
    m_Info.DeviceID = Properties.deviceID;
    m_Info.Name = Properties.deviceName;

    return *this;
}

PhysicalDevice& PhysicalDevice::FindQueueFamily()
{
    if (m_Device == VK_NULL_HANDLE)
    {
        return *this;
    }

    u32 Count { 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &Count, nullptr);

    Array<VkQueueFamilyProperties> Properties;
    Properties.Resize(Count);
    vkGetPhysicalDeviceQueueFamilyProperties(m_Device, &Count, Properties.Data());

    u32 Index { 0 };
    for (const VkQueueFamilyProperties& Property : Properties)
    {
        if (Property.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            m_QueueFamily.Graphics = Index;
        }

        Index++;
    }

    return *this;
}

}
}
}

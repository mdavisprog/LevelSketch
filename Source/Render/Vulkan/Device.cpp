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

#include "Device.hpp"
#include "../../Core/Console.hpp"
#include "Surface.hpp"
#include "SwapChain.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

Device::Device()
{
}

bool Device::Initialize(VkInstance Instance, const Surface& Surface_, const Array<const char*>& Layers)
{
    if (Instance == VK_NULL_HANDLE)
    {
        Core::Console::Error("Failed to initialize device. Invalid VkInstance.");
        return false;
    }

    if (!Surface_.IsValid())
    {
        Core::Console::Error("Failed to initialize device. Invalid surface.");
        return false;
    }

    if (!SelectBestPhysicalDevice(Instance, Surface_))
    {
        return false;
    }

    if (!m_LogicalDevice.Initialize(m_PhysicalDevice, Layers))
    {
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

    return true;
}

void Device::Shutdown()
{
    m_LogicalDevice.Shutdown();
}

bool Device::IsValid() const
{
    return m_PhysicalDevice.IsValid() && m_LogicalDevice.IsValid();
}

bool Device::SelectBestPhysicalDevice(VkInstance Instance, const Surface& Surface_)
{
    Array<PhysicalDevice> Devices { PhysicalDevice::GetDevices(Instance, Surface_) };

    if (Devices.IsEmpty())
    {
        Core::Console::Error("Failed to find a physical device.");
        return false;
    }

    for (const PhysicalDevice& Device : Devices)
    {
        bool IsValid {
            Device.QueueFamilyIndex().IsComplete() &&
            Device.AreRequiredExtensionsSupported()
        };

        SwapChain::SupportDetails SwapChainDetails { SwapChain::GatherDetails(Device, Surface_) };
        IsValid &= SwapChainDetails.IsValid();

        if (IsValid)
        {
            m_PhysicalDevice = Device;
            break;
        }
    }

    if (m_PhysicalDevice.IsValid())
    {
        Core::Console::WriteLine("Selected Device:");
        m_PhysicalDevice.PrintInfo();
    }
    else
    {
        Core::Console::Error("Failed to find a device that supports required queues and extensions.");
    }

    return m_PhysicalDevice.IsValid();
}

const LogicalDevice& Device::GetLogicalDevice() const
{
    return m_LogicalDevice;
}

const PhysicalDevice& Device::GetPhysicalDevice() const
{
    return m_PhysicalDevice;
}

}
}
}

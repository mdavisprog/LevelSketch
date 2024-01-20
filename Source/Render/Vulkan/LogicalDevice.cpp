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

#include "LogicalDevice.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Containers/Array.hpp"
#include "Errors.hpp"
#include "PhysicalDevice.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

LogicalDevice::LogicalDevice()
{
}

bool LogicalDevice::Initialize(const PhysicalDevice& PhysDevice, const Array<const char*>& Layers)
{
    Array<u32> UniqueIndices;
    for (u32 Index : PhysDevice.QueueFamilyIndex().Indices())
    {
        if (!UniqueIndices.Contains(Index))
        {
            UniqueIndices.Push(Index);
        }
    }

    const f32 QueuePriority { 1.0f };
    Array<VkDeviceQueueCreateInfo> QueueInfos;
    for (u32 Index : UniqueIndices)
    {
        VkDeviceQueueCreateInfo QueueInfo {};
        QueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueInfo.queueFamilyIndex = Index;
        QueueInfo.queueCount = 1;
        QueueInfo.pQueuePriorities = &QueuePriority;
        QueueInfos.Push(QueueInfo);
    }

    VkPhysicalDeviceFeatures Features {};

    VkDeviceCreateInfo DeviceInfo {};
    DeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    DeviceInfo.pQueueCreateInfos = QueueInfos.Data();
    DeviceInfo.queueCreateInfoCount = static_cast<u32>(QueueInfos.Size());
    DeviceInfo.pEnabledFeatures = &Features;
    DeviceInfo.enabledLayerCount = static_cast<u32>(Layers.Size());
    DeviceInfo.ppEnabledLayerNames = Layers.Data();
    DeviceInfo.enabledExtensionCount = static_cast<u32>(PhysicalDevice::s_RequiredExtensions.Size());
    DeviceInfo.ppEnabledExtensionNames = PhysicalDevice::s_RequiredExtensions.Data();

    VkResult Result { vkCreateDevice(PhysDevice.Handle(), &DeviceInfo, nullptr, &m_Device) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create logical device. Error: %s", Errors::ToString(Result));
        return false;
    }

    return m_Device != VK_NULL_HANDLE;
}

void LogicalDevice::Shutdown()
{
    if (m_Device != nullptr)
    {
        vkDestroyDevice(m_Device, nullptr);
        m_Device = VK_NULL_HANDLE;
    }
}

bool LogicalDevice::IsValid() const
{
    return m_Device != nullptr;
}

VkDevice LogicalDevice::Handle() const
{
    return m_Device;
}

}
}
}

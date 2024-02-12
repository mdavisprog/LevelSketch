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
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "Queue.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

Device::Device()
{
}

Device::~Device()
{
}

bool Device::Initialize(VkInstance Instance, Surface const* Surface_, const Array<const char*>& Layers)
{
    m_PhysicalDevice = PhysicalDevice::BestDevice(Instance, Surface_);

    if (m_PhysicalDevice == nullptr)
    {
        return false;
    }

    Core::Console::WriteLine("Physical Device: %s", m_PhysicalDevice->GetInfo().Name.Data());

    m_LogicalDevice = UniquePtr<LogicalDevice>::New();

    if (!m_LogicalDevice->Initialize(m_PhysicalDevice.Get(), Layers))
    {
        return false;
    }

    m_GraphicsQueue = UniquePtr<Queue>::New();
    if (!m_GraphicsQueue->Initialize(m_LogicalDevice.Get(), m_PhysicalDevice->GetQueueFamily().Graphics.Value()))
    {
        Core::Console::Error("Failed to initialize graphics queue.");
        return false;
    }

    m_PresentQueue = UniquePtr<Queue>::New();
    if (!m_PresentQueue->Initialize(m_LogicalDevice.Get(), m_PhysicalDevice->GetQueueFamily().Present.Value()))
    {
        Core::Console::Error("Failed to initialize present queue.");
        return false;
    }

    return true;
}

void Device::Shutdown()
{
    m_LogicalDevice->Shutdown();
}

void Device::WaitForIdle() const
{
    vkDeviceWaitIdle(m_LogicalDevice->Get());
}

LogicalDevice const* Device::GetLogicalDevice() const
{
    return m_LogicalDevice.Get();
}

PhysicalDevice const* Device::GetPhysicalDevice() const
{
    return m_PhysicalDevice.Get();
}

Queue const* Device::GraphicsQueue() const
{
    return m_GraphicsQueue.Get();
}

Queue const* Device::PresentQueue() const
{
    return m_PresentQueue.Get();
}

}
}
}

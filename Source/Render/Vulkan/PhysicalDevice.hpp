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

#pragma once

#include "../../Core/Containers/String.hpp"
#include "../../Core/Optional.hpp"
#include "vulkan/vulkan.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

class Surface;

class PhysicalDevice
{
public:
    class QueueFamily
    {
        friend class PhysicalDevice;
    
    public:
        bool HasGraphics() const { return m_Graphics.HasValue(); }
        u32 Graphics() const { return m_Graphics.Value(); }

        bool HasPresent() const { return m_Present.HasValue(); }
        u32 Present() const { return m_Present.Value(); }

        bool IsComplete() const { return HasGraphics() && HasPresent(); }

        Array<u32> Indices() const { return {Graphics(), Present()}; }

    private:
        Optional<u32> m_Graphics {};
        Optional<u32> m_Present {};
    };

    static Array<PhysicalDevice> GetDevices(VkInstance Instance, const Surface& Surface_);

    PhysicalDevice();

    void PrintInfo() const;
    VkPhysicalDevice Handle() const;
    const QueueFamily& QueueFamilyIndex() const;

private:
    struct Info
    {
        u32 APIVersion_Major { 0 };
        u32 APIVersion_Minor { 0 };
        u32 APIVersion_Patch { 0 };
        u32 DriverVersion { 0 };
        u32 VendorID { 0 };
        u32 DeviceID { 0 };
        String Name {};
    };

    PhysicalDevice(VkPhysicalDevice Device);

    PhysicalDevice& GatherInfo();
    PhysicalDevice& FindQueueFamily(const Surface& Surface_);

    VkPhysicalDevice m_Device { VK_NULL_HANDLE };
    QueueFamily m_QueueFamily {};
    Info m_Info {};
};

}
}
}

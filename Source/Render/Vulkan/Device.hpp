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

#include "CommandPool.hpp"
#include "LogicalDevice.hpp"
#include "PhysicalDevice.hpp"
#include "Queue.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

class Surface;

class Device
{
public:
    Device();

    bool Initialize(VkInstance Instance, const Surface& Surface_, const Array<const char*>& Layers);
    void Shutdown();
    void WaitForIdle() const;

    bool IsValid() const;

    const LogicalDevice& GetLogicalDevice() const;
    const PhysicalDevice& GetPhysicalDevice() const;
    const Queue& GraphicsQueue() const;
    const Queue& PresentQueue() const;
    const CommandPool& GetCommandPool() const;

private:
    bool SelectBestPhysicalDevice(VkInstance Instance, const Surface& Surface_);

    LogicalDevice m_LogicalDevice {};
    PhysicalDevice m_PhysicalDevice {};
    Queue m_GraphicsQueue {};
    Queue m_PresentQueue {};
    CommandPool m_CommandPool {};
};

}
}
}

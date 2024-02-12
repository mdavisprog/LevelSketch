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

#include "../../Core/Containers/Forwards.hpp"
#include "../../Core/Memory/UniquePtr.hpp"

#include <vulkan/vulkan.h>

namespace LevelSketch
{

namespace Platform
{
class Window;
}

namespace Render
{
namespace Vulkan
{

class LogicalDevice;
class PhysicalDevice;
class Queue;
class Surface;

class Device final
{
public:
    Device();
    ~Device();

    bool Initialize(VkInstance Instance, Surface const* Surface_, const Array<const char*>& Layers);
    void Shutdown();
    void WaitForIdle() const;

    LogicalDevice const* GetLogicalDevice() const;
    PhysicalDevice const* GetPhysicalDevice() const;
    Queue const* GraphicsQueue() const;
    Queue const* PresentQueue() const;

private:
    UniquePtr<LogicalDevice> m_LogicalDevice { nullptr };
    UniquePtr<PhysicalDevice> m_PhysicalDevice { nullptr };
    UniquePtr<Queue> m_GraphicsQueue { nullptr };
    UniquePtr<Queue> m_PresentQueue { nullptr };
};

}
}
}

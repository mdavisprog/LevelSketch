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

#include "../../Core/Containers/Array.hpp"
#include "CommandBuffer.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

class Device;

class CommandPool
{
public:
    CommandPool();

    bool Initialize(const Device& Device_);
    bool InitializeBuffers(const Device& Device_, u64 Count);
    void Shutdown(const Device& Device_);

    bool IsValid() const;
    VkCommandPool Handle() const;
    const CommandBuffer& Buffer(u64 Index) const;

private:
    VkCommandPool m_Handle { VK_NULL_HANDLE };
    Array<CommandBuffer> m_CommandBuffers {};
};

}
}
}
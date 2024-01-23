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

#include "RenderBuffer.hpp"
#include "../../Core/Console.hpp"
#include "Device.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

RenderBuffer::RenderBuffer()
{
}

bool RenderBuffer::Initialize(const Device& Device_, u64 VertexSize, u64 IndexSize)
{
    if (!InitializeBuffer(m_VertexBuffer, Device_, VertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT))
    {
        Core::Console::Error("Failed to initialize vertex buffer.");
        return false;
    }

    if (!InitializeBuffer(m_IndexBuffer, Device_, IndexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
    {
        Core::Console::Error("Failed to initialize index buffer.");
        return false;
    }

    return true;
}

void RenderBuffer::Shutdown(const Device& Device_)
{
    m_VertexBuffer.Shutdown(Device_);
    m_IndexBuffer.Shutdown(Device_);
}

const Buffer& RenderBuffer::VertexBuffer() const
{
    return m_VertexBuffer;
}

const Buffer& RenderBuffer::IndexBuffer() const
{
    return m_IndexBuffer;
}

bool RenderBuffer::InitializeBuffer(Buffer& Buf, const Device& Device_, u64 Size, VkBufferUsageFlags Usage) const
{
    if (Buf.IsValid())
    {
        return true;
    }

    if (!Buf.Initialize(
        Device_,
        Size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | Usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
    {
        return false;
    }

    return true;
}

}
}
}

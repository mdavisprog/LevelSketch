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

#include "VertexBuffer.hpp"
#include "../../Core/Console.hpp"
#include "../VertexBufferDescription.hpp"
#include "Buffer.hpp"
#include "Device.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

u32 VertexBuffer::s_ID { 0 };

VertexBuffer::VertexBuffer()
{
}

bool VertexBuffer::Initialize(Device const* Device_, const VertexBufferDescription& Description)
{
    m_VertexBuffer = UniquePtr<Buffer>::New();
    if (!InitializeBuffer(m_VertexBuffer, Device_, Description.VertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT))
    {
        Core::Console::Error("Failed to initialize vertex buffer.");
        return false;
    }

    m_IndexBuffer = UniquePtr<Buffer>::New();
    if (!InitializeBuffer(m_IndexBuffer, Device_, Description.IndexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
    {
        Core::Console::Error("Failed to initialize index buffer.");
        return false;
    }

    m_IndexType = Description.IndexFormat == IndexFormatType::U16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
    m_ID = ++s_ID;

    return true;
}

void VertexBuffer::Shutdown(Device const* Device_)
{
    m_VertexBuffer->Shutdown(Device_);
    m_VertexBuffer = nullptr;

    m_IndexBuffer->Shutdown(Device_);
    m_IndexBuffer = nullptr;
}

Buffer const* VertexBuffer::GetVertexBuffer() const
{
    return m_VertexBuffer.Get();
}

Buffer const* VertexBuffer::GetIndexBuffer() const
{
    return m_IndexBuffer.Get();
}

VkIndexType VertexBuffer::IndexType() const
{
    return m_IndexType;
}

u32 VertexBuffer::ID() const
{
    return m_ID;
}

bool VertexBuffer::InitializeBuffer(const UniquePtr<Buffer>& Buffer_,
    Device const* Device_,
    u64 Size,
    VkBufferUsageFlags Usage) const
{
    if (!Buffer_->Initialize(Device_,
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

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
#include "../VertexDataDescription.hpp"
#include "Device.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

u32 VertexBuffer::s_ID { 0 };

VertexBuffer::VertexBuffer()
{
}

bool VertexBuffer::Initialize(Device const* Device_, const VertexBufferDescription& Description)
{
    m_VertexBuffer = [Device_->Get() newBufferWithLength:Description.VertexBufferSize
                                                 options:MTLResourceStorageModeShared];

    if (m_VertexBuffer == nullptr)
    {
        Core::Console::Error("Failed to create vertex buffer.");
        return false;
    }

    m_IndexBuffer = [Device_->Get() newBufferWithLength:Description.IndexBufferSize
                                                options:MTLResourceStorageModeShared];

    if (m_IndexBuffer == nullptr)
    {
        Core::Console::Error("Failed to create index buffer.");
        return false;
    }

    m_IndexType = Description.IndexFormat == IndexFormatType::U16 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
    m_Stride = Description.Stride;
    m_ID = ++s_ID;

    return true;
}

bool VertexBuffer::Upload(const VertexDataDescription& Description)
{
    std::memcpy(m_VertexBuffer.contents, Description.VertexData, Description.VertexDataSize);
    std::memcpy(m_IndexBuffer.contents, Description.IndexData, Description.IndexDataSize);
    return true;
}

id<MTLBuffer> VertexBuffer::GetVertexBuffer() const
{
    return m_VertexBuffer;
}

id<MTLBuffer> VertexBuffer::GetIndexBuffer() const
{
    return m_IndexBuffer;
}

MTLIndexType VertexBuffer::IndexType() const
{
    return m_IndexType;
}

u64 VertexBuffer::IndexTypeSize() const
{
    return m_IndexType == MTLIndexTypeUInt16 ? sizeof(u16) : sizeof(u32);
}

u64 VertexBuffer::Stride() const
{
    return m_Stride;
}

u32 VertexBuffer::ID() const
{
    return m_ID;
}

}
}
}

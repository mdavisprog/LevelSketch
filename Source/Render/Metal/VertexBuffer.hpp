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

#include "../../Core/Types.hpp"

@protocol MTLBuffer;

namespace LevelSketch
{
namespace Render
{

struct VertexBufferDescription;
struct VertexDataDescription;

namespace Metal
{

class Device;

class VertexBuffer
{
private:
    static u32 s_ID;

public:
    VertexBuffer();

    bool Initialize(Device const* Device_, const VertexBufferDescription& Description);
    bool Upload(const VertexDataDescription& Description);

    id<MTLBuffer> GetVertexBuffer() const;
    id<MTLBuffer> GetIndexBuffer() const;

    u32 IndexType() const;
    u32 ID() const;

private:
    id<MTLBuffer> m_VertexBuffer { nullptr };
    id<MTLBuffer> m_IndexBuffer { nullptr };
    u32 m_IndexType { 0 };
    u32 m_ID { 0 };
};

}
}
}

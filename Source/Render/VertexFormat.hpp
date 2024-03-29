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

#include "../Core/Types.hpp"

namespace LevelSketch
{
namespace Render
{

enum class VertexFormat
{
    Byte,
    Byte2,
    Byte4,
    Float,
    Float2,
    Float3,
    Float4
};

static inline u64 VertexFormatSize(VertexFormat Format)
{
    switch (Format)
    {
    case VertexFormat::Byte: return sizeof(u8);
    case VertexFormat::Byte2: return sizeof(u8) * 2;
    case VertexFormat::Byte4: return sizeof(u8) * 4;
    case VertexFormat::Float: return sizeof(f32);
    case VertexFormat::Float2: return sizeof(f32) * 2;
    case VertexFormat::Float3: return sizeof(f32) * 3;
    case VertexFormat::Float4: return sizeof(f32) * 4;
    default: break;
    }

    return 0;
}

}
}

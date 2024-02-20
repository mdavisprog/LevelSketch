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

#include <type_traits>

namespace LevelSketch
{
namespace Render
{

template<typename T>
struct Handle
{
private:
    static u32 s_ID;

public:
    static T Acquire()
    {
        static_assert(std::is_base_of_v<Handle, T>, "Type T must be derived from Handle.");
        T Result;
        Result.m_ID = ++s_ID;
        return Result;
    }

    static T ToHandle(u32 ID)
    {
        static_assert(std::is_base_of_v<Handle, T>, "Type T must be derived from Handle.");
        T Result;
        Result.m_ID = ID;
        return Result;
    }

    Handle()
    {
    }

    bool operator==(const Handle<T>& Other) const
    {
        return m_ID == Other.m_ID;
    }

    bool operator!=(const Handle<T>& Other) const
    {
        return m_ID != Other.m_ID;
    }

    u32 ID() const
    {
        return m_ID;
    }

    bool IsValid() const
    {
        return m_ID != 0;
    }

private:
    u32 m_ID { 0 };
};

template<typename T>
u32 Handle<T>::s_ID { 0 };

struct GraphicsPipelineHandle : public Handle<GraphicsPipelineHandle>
{
};

struct TextureHandle : public Handle<TextureHandle>
{
};

struct VertexBufferHandle : public Handle<VertexBufferHandle>
{
};

}
}

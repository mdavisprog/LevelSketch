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

#include "Assert.hpp"
#include "Types.hpp"

namespace LevelSketch
{
namespace Core
{

template<typename T>
class Optional
{
public:
    Optional()
    {
    }

    Optional(const Optional<T>& Other)
    {
        Construct(Other.Value());
    }

    Optional<T>& operator=(const Optional<T>& Other)
    {
        Clear();
        Construct(Other.Value());
        return *this;
    }

    Optional(const T& Value)
    {
        Construct(Value);
    }

    Optional<T>& operator=(const T& Value)
    {
        Clear();
        Construct(Value);
        return *this;
    }

    ~Optional()
    {
        Clear();
    }

    bool HasValue() const
    {
        return m_HasValue;
    }

    void Clear()
    {
        if (m_HasValue)
        {
            Value().~T();
            m_HasValue = false;
        }
    }

    T& Value()
    {
        LS_ASSERT(m_HasValue);
        return *reinterpret_cast<T*>(&m_Value);
    }

    T const& Value() const
    {
        LS_ASSERT(m_HasValue);
        return *reinterpret_cast<T const*>(&m_Value);
    }

private:
    void Construct(const T& Value)
    {
        LS_ASSERT(!m_HasValue);
        m_HasValue = true;
        new(m_Value) T(Value);
    }

    alignas(T) u8 m_Value[sizeof(T)];
    bool m_HasValue { false };
};

}

template<typename T>
using Optional = Core::Optional<T>;

}

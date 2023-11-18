/**

MIT License

Copyright (c) 2023 Mitchell Davis <mdavisprog@gmail.com>

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

#include "../Types.hpp"

#include <cstring>

namespace Core
{
namespace Containers
{

template<typename T>
class Array
{
public:
    Array()
    {
    }

    Array(const Array& Other)
    {
        Copy(Other);
    }

    Array(const Array&& Other)
    {
    }

    ~Array()
    {
        Clear();
    }

    u64 Capacity() const
    {
        return m_Capacity;
    }

    u64 Size() const
    {
        return m_Size;
    }

    Array& Copy(const Array& Other)
    {
        m_Capacity = Other.Capacity();
        m_Size = Other.Size();

        if (m_Capacity > 0)
        {
            m_Data = new T[m_Capacity];

            for (u64 I = 0; I < m_Capacity; I++)
            {
                m_Data[I] = Other.m_Data[I];
            }
        }

        return *this;
    }

    Array& Clear()
    {
        if (m_Data != nullptr)
        {
            delete[] m_Data;
        }

        m_Capacity = 0;
        m_Size = 0;
        m_Data = nullptr;

        return *this;
    }

private:
    T* m_Data { nullptr };
    u64 m_Capacity { 0 };
    u64 m_Size { 0 };
};

}
}

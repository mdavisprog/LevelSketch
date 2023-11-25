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

#include "../Assert.hpp"
#include "../Math/Math.hpp"
#include "../Types.hpp"

#include <initializer_list>
#include <utility>

#define ARRAY_GRANULARITY 16

namespace LevelSketch
{
namespace Core
{
namespace Containers
{

template<typename T>
class Array
{
public:
    static const u64 Granularity { 16 };

    Array()
    {
    }

    Array(const Array& Other)
    {
        Copy(Other);
    }

    Array(Array&& Other)
    {
        Move(std::move(Other));
    }

    Array(const std::initializer_list<T>& List)
    {
        Resize(List.size());

        for (const T& Item : List)
        {
            Push(Item);
        }
    }

    ~Array()
    {
        Clear();
    }

    Array& operator=(const Array& Other)
    {
        Copy(Other);
        return *this;
    }

    Array& operator=(Array&& Other)
    {
        return Move(std::move(Other));
    }

    T& operator[](u64 Index)
    {
        LS_ASSERT(m_Size > 0);
        LS_ASSERT(Index < m_Size);
        return m_Data[Index];
    }

    const T& operator[](u64 Index) const
    {
        LS_ASSERT(m_Size > 0);
        LS_ASSERT(Index < m_Size);
        return m_Data[Index];
    }

    u64 Capacity() const
    {
        return m_Capacity;
    }

    u64 Size() const
    {
        return m_Size;
    }

    T* Data()
    {
        return m_Data;
    }

    const T* Data() const
    {
        return m_Data;
    }

    Array& Copy(const Array& Other)
    {
        Clear();

        m_Capacity = Other.Capacity();
        m_Size = Other.Size();

        if (m_Capacity > 0)
        {
            m_Data = Allocate(m_Capacity);

            for (u64 I = 0; I < m_Size; I++)
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
            DestructorArray<T>(m_Data, m_Size);
            Free();
        }

        m_Capacity = 0;
        m_Size = 0;

        return *this;
    }

    Array& Resize(u64 Capacity)
    {
        if (Capacity == 0)
        {
            Clear();
            return *this;
        }

        if (m_Capacity == Capacity)
        {
            return *this;
        }

        T* Temp { m_Data };

        m_Capacity = Capacity;
        m_Size = m_Capacity < m_Size ? m_Capacity : m_Size;

        m_Data = Allocate(m_Capacity);

        for (u64 I = 0; I < Math::Min(m_Size, m_Capacity); I++)
        {
            if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
            {
                m_Data[I] = std::move(Temp[I]);
            }
            else
            {
                m_Data[I] = Temp[I];
            }
        }

        Free(Temp);

        return *this;
    }

    Array& Push(const T& Value)
    {
        ConditionalGrow();

        new(static_cast<void*>(&m_Data[m_Size])) T(Value);
        m_Size++;

        return *this;
    }

    Array& Push(T&& Value)
    {
        ConditionalGrow();

        new(static_cast<void*>(&m_Data[m_Size])) T(std::move(Value));
        m_Size++;

        return *this;
    }

    Array& Pop()
    {
        if (m_Size == 0)
        {
            return *this;
        }

        Remove(m_Size - 1);

        return *this;
    }

    bool Remove(u64 Index)
    {
        if (m_Data == nullptr)
        {
            return false;
        }

        if (Index >= m_Size)
        {
            return false;
        }

        Destructor<T>(m_Data);
        m_Size--;

        for (u64 I = Index; I < m_Size; I++)
        {
            m_Data[Index] = m_Data[Index + 1];
        }

        return true;
    }

    T* begin()
    {
        return m_Data;
    }

    T* end()
    {
        return m_Data + m_Size;
    }

    const T* begin() const
    {
        return m_Data;
    }

    const T* end() const
    {
        return m_Data + m_Size;
    }

private:
    template<typename U>
    static void Destructor(void* Ptr)
    {
        static_cast<U*>(Ptr)->~T();
    }

    template<typename U>
    static void DestructorArray(void* Ptr, u64 Size)
    {
        for (u64 I = 0; I < Size; I++)
        {
            U& Item { static_cast<U*>(Ptr)[I] };
            Destructor<U>(static_cast<void*>(&Item));
        }
    }

    static T* Allocate(u64 Size)
    {
        return static_cast<T*>(malloc(sizeof(T) * Size));
    }

    static void Free(void* Ptr)
    {
        if (Ptr != nullptr)
        {
            free(Ptr);
        }
    }

    Array& Free()
    {
        Free(m_Data);
        m_Data = nullptr;
        return *this;
    }

    Array& ConditionalGrow()
    {
        if (m_Data == nullptr || m_Capacity == m_Size)
        {
            Resize(GrowSize());
        }

        return *this;
    }

    u64 GrowSize() const
    {
        const u64 NewSize { m_Capacity + ARRAY_GRANULARITY };
        return NewSize - NewSize % ARRAY_GRANULARITY;
    }

    Array& Move(Array&& Other)
    {
        Clear();
        m_Capacity = Other.Capacity();
        m_Size = Other.Size();
        m_Data = std::move(Other.m_Data);

        Other.m_Capacity = 0;
        Other.m_Size = 0;
        Other.m_Data = nullptr;

        return *this;
    }

    T* m_Data { nullptr };
    u64 m_Capacity { 0 };
    u64 m_Size { 0 };
};

}
}
}

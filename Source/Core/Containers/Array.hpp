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

#include "../Assert.hpp"
#include "../Math/Math.hpp"
#include "../Types.hpp"

#include <cstdlib>
#include <initializer_list>
#include <memory>
#include <new>
#include <utility>

#define ARRAY_GRANULARITY 16

#ifndef ARRAY_COUNT
#define ARRAY_COUNT(X) (sizeof(X) / sizeof(X[0]))
#endif

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

    Array(Array&& Other) noexcept
    {
        Move(std::move(Other));
    }

    Array(const std::initializer_list<T>& List)
    {
        Reserve(List.size());

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

    Array& operator=(Array&& Other) noexcept
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

    Array<T> operator+(const Array<T>& Other) const
    {
        Array<T> Result;
        Result.Reserve(Size() + Other.Size());

        for (const T& Item : *this)
        {
            Result.Push(Item);
        }

        for (const T& Item : Other)
        {
            Result.Push(Item);
        }

        return Result;
    }

    Array<T>& operator+=(const Array<T>& Other)
    {
        ConditionalGrow(Other.Size());

        // Push items from one item to the other to properly call ctor.
        for (const T& Item : Other)
        {
            Push(Item);
        }

        return *this;
    }

    u64 Capacity() const
    {
        return m_Capacity;
    }

    u64 Size() const
    {
        return m_Size;
    }

    bool IsEmpty() const
    {
        return m_Size == 0;
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

            // FIXME: Should the Allocate function do an in-place new for
            // each element to safely construct each object? Will do
            // that here before a copy.

            for (u64 I = 0; I < m_Size; I++)
            {
                new (std::addressof(m_Data[I])) T;
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

    Array& Resize(u64 Size)
    {
        if (m_Size == Size)
        {
            return *this;
        }

        if (Size < m_Size)
        {
            RemoveRange(Size, m_Size - Size);
        }
        else
        {
            ConditionalGrow(Size - m_Size);

            // FIXME: Should the Allocate function do an in-place new for
            // each element to safely construct each object? Will do
            // that here after growing.

            for (u64 I = m_Size; I < Size - m_Size; I++)
            {
                new (std::addressof(m_Data[I])) T;
            }
        }

        m_Size = Size;

        return *this;
    }

    Array& AddZeroed(u64 Count = 1)
    {
        Resize(m_Size + Count);
        return *this;
    }

    Array& Reserve(u64 Capacity)
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
                new (std::addressof(m_Data[I])) T(std::move(Temp[I]));
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

        new (std::addressof(m_Data[m_Size])) T(Value);
        m_Size++;

        return *this;
    }

    Array& Push(T&& Value)
    {
        ConditionalGrow();

        new (std::addressof(m_Data[m_Size])) T(std::move(Value));
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
        return RemoveRange(Index);
    }

    bool Remove(const T& Item)
    {
        for (u64 I = 0; I < m_Size; I++)
        {
            if (m_Data[I] == Item)
            {
                return RemoveRange(I);
            }
        }

        return false;
    }

    bool RemoveRange(u64 Index, u64 Count = 1)
    {
        if (Count == 0)
        {
            return false;
        }

        if (Index >= m_Size)
        {
            return false;
        }

        Count = Math::Min(Count, m_Size - Index);
        const u64 Last { Index + Count };

        // Call destructor on each element.
        for (u64 I = Index; I < Last; I++)
        {
            Destructor<T>(std::addressof(m_Data[I]));
        }

        // Copy/Move remaining elements down to fill in destroyed slots.
        for (u64 I = Index, J = Last; J < m_Size; I++, J++)
        {
            if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
            {
                m_Data[I] = std::move(m_Data[J]);
            }
            else
            {
                m_Data[I] = m_Data[J];
            }
        }

        m_Size -= Count;

        return true;
    }

    bool Contains(const T& Value) const
    {
        if (IsEmpty())
        {
            return false;
        }

        for (u64 I = 0; I < m_Size; I++)
        {
            if (m_Data[I] == Value)
            {
                return true;
            }
        }

        return false;
    }

    T& Back()
    {
        LS_ASSERT(!IsEmpty());
        return m_Data[Size() - 1];
    }

    T const& Back() const
    {
        LS_ASSERT(!IsEmpty());
        return m_Data[Size() - 1];
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
            Destructor<U>(std::addressof(Item));
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

    Array& ConditionalGrow(u64 AdditionalSpace = 0)
    {
        if (m_Data == nullptr || m_Capacity == m_Size || m_Size + AdditionalSpace >= m_Capacity)
        {
            const u64 Capacity { Math::Max(m_Capacity + AdditionalSpace, GrowSize()) };
            Reserve(Capacity);
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

template<typename T>
bool operator==(const Array<T>& A, const Array<T>& B)
{
    if (A.Size() != B.Size())
    {
        return false;
    }

    for (u64 I = 0; I < A.Size(); I++)
    {
        if (A[I] != B[I])
        {
            return false;
        }
    }

    return true;
}

template<typename T>
bool operator!=(const Array<T>& A, const Array<T>& B)
{
    return !(A == B);
}

}
}

template<typename T>
using Array = Core::Containers::Array<T>;

}

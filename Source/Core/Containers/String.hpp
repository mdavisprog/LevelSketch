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

#include "Array.hpp"

#include <cstring>

namespace LevelSketch
{
namespace Core
{
namespace Containers
{

template<typename T>
class TString
{
public:
    static u64 NPOS;

    static u64 LengthFromPtr(const T* Data)
    {
        u64 Result { 0 };

        const T* Ptr { Data };
        while (Ptr != nullptr && *Ptr != 0)
        {
            Result++;
            Ptr++;
        }

        return Result;
    }

    TString()
    {
        Copy(nullptr);
    }

    TString(const T* Data)
    {
        Copy(Data);
    }

    ~TString()
    {
        Clear();
    }

    TString<T>& operator=(const T* Data)
    {
        Copy(Data);
        return *this;
    }

    T& operator[](u64 Index)
    {
        return m_Data[Index];
    }

    T const& operator[](u64 Index) const
    {
        return m_Data[Index];
    }

    TString<T> operator+(const TString<T>& Other) const
    {
        TString<T> Result;

        Result.m_Data = m_Data;
        Result.m_Data.Pop(); // Remove terminator.
        Result.m_Data += Other.m_Data;

        return Result;
    }

    TString<T>& operator+=(const TString<T>& Other)
    {
        m_Data.Pop(); // Remove terminator.
        m_Data += Other.m_Data;
        return *this;
    }

    TString<T>& operator+=(const T* Other)
    {
        return Append(Other);
    }

    T* Data()
    {
        return m_Data.Data();
    }

    T const* Data() const
    {
        return m_Data.Data();
    }

    u64 Length() const
    {
        if (m_Data.IsEmpty())
        {
            return 0;
        }

        return m_Data.Size() - 1;
    }

    u64 Size() const
    {
        return Length();
    }

    u64 Capacity() const
    {
        return m_Data.Capacity();
    }

    TString<T>& Clear()
    {
        m_Data.Clear();
        return *this;
    }

    bool IsEmpty() const
    {
        return Length() == 0;
    }

    TString<T>& Reserve(u64 Size)
    {
        m_Data.Reserve(Size);
        return *this;
    }

    TString<T>& Resize(u64 Size)
    {
        m_Data.Resize(Size);
        return *this;
    }

    u64 Find(T Ch, u64 Pos = 0) const
    {
        for (u64 I = Pos; I <= Length(); I++)
        {
            if (m_Data[I] == Ch)
            {
                return I;
            }
        }

        return NPOS;
    }

    u64 RFind(T Ch, u64 Pos = NPOS) const
    {
        u64 I = (Pos == NPOS ? Length() : Pos);
        while (true)
        {
            if (m_Data[I] == Ch)
            {
                return I;
            }

            if (I == 0)
            {
                break;
            }

            I--;
        }

        return NPOS;
    }

    TString<T> Sub(u64 Pos, u64 Count = NPOS) const
    {
        Pos = Pos > Length() ? Length() : Pos;
        Count = Count == NPOS ? Length() - Pos : Count;
        Count = Length() - Pos < Count ? Length() - Pos : Count;

        TString<T> Result;

        if (Count == 0)
        {
            return Result;
        }

        Result.Resize(Count + 1);

        std::memcpy(Result.Data(), Data() + Pos, Count);
        Result[Count] = 0;

        return Result;
    }

    TString<T>& Append(const T* Other)
    {
        const u64 OtherLength { LengthFromPtr(Other) };
        m_Data.Pop();
        const u64 Size { m_Data.Size() };
        m_Data.Resize(m_Data.Size() + OtherLength);
        std::memcpy(m_Data.Data() + Size, Other, OtherLength);
        m_Data.Push(0);
        return *this;
    }

private:
    TString<T>& Copy(const T* Data)
    {
        Clear();
        return Copy(Data, LengthFromPtr(Data));
    }

    TString<T>& Copy(const T* Data, u64 Length)
    {
        Clear();
        m_Data.Resize(Length + 1);
        std::memcpy(m_Data.Data(), Data, sizeof(T) * (u32)Length);
        m_Data[Length] = 0;
        return *this;
    }

    Array<T> m_Data {};
};

template<typename T>
u64 TString<T>::NPOS { static_cast<u64>(-1) };

template<typename T>
static bool operator==(const TString<T>& A, const T* B)
{
    if (A.Length() != TString<T>::LengthFromPtr(B))
    {
        return false;
    }

    for (u64 I = 0; I < A.Length(); I++)
    {
        if (A[I] != B[I])
        {
            return false;
        }
    }

    return true;
}

template<typename T>
static bool operator!=(const TString<T>& A, const T* B)
{
    return !(A == B);
}

typedef TString<char> String;
typedef TString<wchar_t> WString;

String ToString(const WString& Value);
WString ToWString(const String& Value);

}
}

using String = Core::Containers::String;
using WString = Core::Containers::WString;

}

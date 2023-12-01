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
        return m_Data.IsEmpty();
    }

    TString<T>& Reserve(u64 Size)
    {
        m_Data.Reserve(Size);
        return *this;
    }

private:
    TString<T>& Copy(const T* Data)
    {
        Clear();

        if (Data == nullptr)
        {
            return *this;
        }

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

}

// Use an alias here so that the String class can be referred to as Core::String
using String = Containers::String;
using WString = Containers::WString;

}
}

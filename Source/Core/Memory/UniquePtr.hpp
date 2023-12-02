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

#include <cstddef>
#include <type_traits>
#include <utility>

namespace LevelSketch
{
namespace Core
{
namespace Memory
{

template<typename T>
class UniquePtr
{
public:
    template<typename... Arguments>
    static UniquePtr<T> New(Arguments&&... Args)
    {
        return UniquePtr<T>(new T(std::forward<Arguments>(Args)...));
    }

    static UniquePtr<T> Adopt(T* Data)
    {
        return UniquePtr<T>(Data);
    }

    UniquePtr(const UniquePtr<T>&) = delete;
    UniquePtr& operator=(const UniquePtr<T>&) = delete;

    UniquePtr()
    {
    }

    UniquePtr(nullptr_t)
    {
    }

    UniquePtr(UniquePtr<T>&& Other) noexcept
    {
        m_Data = Other.Leak();
    }

    template<typename U>
    UniquePtr(UniquePtr<U>&& Other) noexcept
    {
        static_assert(std::is_base_of_v<T, U>, "Trying to create a unique pointer of a derived class that does not inherit from base.");
        m_Data = Other.Leak();
    }

    ~UniquePtr()
    {
        Reset();
    }

    UniquePtr& operator=(UniquePtr<T>&& Other)
    {
        Reset();
        m_Data = Other.Leak();
        return *this;
    }

    template<typename U>
    UniquePtr& operator=(UniquePtr<U>&& Other)
    {
        static_assert(std::is_base_of_v<T, U>, "Trying to create a unique pointer of a derived class that does not inherit from base.");
        Reset();
        m_Data = Other.Leak();
        return *this;
    }

    UniquePtr<T>& operator=(nullptr_t)
    {
        Reset();
        return *this;
    }

    bool operator!() const
    {
        return m_Data == nullptr;
    }

    operator bool() const
    {
        return m_Data != nullptr;
    }

    T& operator*() const
    {
        return *m_Data;
    }

    T* operator->() const
    {
        return m_Data;
    }

    T* Get() const
    {
        return m_Data;
    }

    void Reset()
    {
        if (m_Data != nullptr)
        {
            delete m_Data;
            m_Data = nullptr;
        }
    }

    T* Leak()
    {
        T* Temp { m_Data };
        m_Data = nullptr;
        return Temp;
    }

    bool IsValid() const
    {
        return m_Data != nullptr;
    }

private:
    UniquePtr(T* Data)
        : m_Data(Data)
    {
    }

    T* m_Data { nullptr };
};

template<typename T, typename U>
bool operator==(const UniquePtr<T>& A, const UniquePtr<U>& B)
{
    return A.Get() == B.Get();
}

template<typename T, typename U>
bool operator!=(const UniquePtr<T>& A, const UniquePtr<U>& B)
{
    return A.Get() != B.Get();
}

template<typename T, typename U>
bool operator==(const UniquePtr<T>& A, const U& B)
{
    return A.Get() == B;
}

template<typename T, typename U>
bool operator!=(const UniquePtr<T>& A, const U& B)
{
    return A.Get() != B;
}

template<typename T>
bool operator==(const UniquePtr<T>& A, nullptr_t)
{
    return A.Get() == nullptr;
}

template<typename T>
bool operator!=(const UniquePtr<T>& A, nullptr_t)
{
    return A.Get() != nullptr;
}

}
}
}


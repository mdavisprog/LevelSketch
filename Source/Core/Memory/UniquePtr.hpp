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

    UniquePtr(UniquePtr<T>&& Other) noexcept
    {
        m_Data = Other.m_Data;
        Other.m_Data = nullptr;
    }

    ~UniquePtr()
    {
        Reset();
    }

    UniquePtr& operator=(UniquePtr<T>&& Other)
    {
        Reset();
        m_Data = Other.m_Data;
        Other.m_Data = nullptr;
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

    T& operator*()
    {
        return *m_Data;
    }

    T const& operator*() const
    {
        return *m_Data;
    }

    T* operator->()
    {
        return m_Data;
    }

    T const* operator->() const
    {
        return m_Data;
    }

    T* Get()
    {
        return m_Data;
    }

    T const* Get() const
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

}
}
}


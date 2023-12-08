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
#include "ReferenceCount.hpp"

#include <cstddef>
#include <utility>

namespace LevelSketch
{
namespace Core
{
namespace Memory
{

template<typename T>
class WeakPtr;

template<typename T>
class Shareable;

template<typename T>
class SharedPtr
{
public:
    template<typename... TArguments>
    static SharedPtr<T> New(TArguments&&... Arguments)
    {
        return SharedPtr<T>( new T(Arguments...) );
    }

    SharedPtr()
    {
    }

    SharedPtr(const SharedPtr<T>& Other)
    {
        Copy(Other);
    }

    template<typename U>
    SharedPtr(const SharedPtr<U>& Other)
    {
        Copy<U>(Other);
    }

    SharedPtr(T* Data)
        : m_Data(Data)
    {
        InitializeReferenceCount();

        if constexpr (std::is_base_of<Shareable<T>, T>::value)
        {
            Shareable<T>* Share = reinterpret_cast<Shareable<T>*>(m_Data);
            if (Share != nullptr && !Share->m_Weak.IsValid())
            {
                Share->m_Weak = *this;
            }
        }
    }

    SharedPtr(SharedPtr<T>&& Other)
    {
        Move(std::move(Other));
    }

    template<typename U>
    SharedPtr(SharedPtr<U>&& Other)
    {
        Move<U>(std::move(Other));
    }

    SharedPtr(const WeakPtr<T>& Other)
    {
        if (Other.m_ReferenceCount != nullptr)
        {
            m_Data = Other.m_Data;
            m_ReferenceCount = Other.m_ReferenceCount;
            m_ReferenceCount->Reference();
        }
    }

    ~SharedPtr()
    {
        Dereference();
    }

    SharedPtr<T>& operator=(const SharedPtr<T>& Other)
    {
        Copy(Other);
        return *this;
    }

    template<typename U>
    SharedPtr<T>& operator=(const SharedPtr<U>& Other)
    {
        Copy<U>(Other);
        return *this;
    }

    SharedPtr<T>& operator=(SharedPtr<T>&& Other)
    {
        Move(std::move(Other));
        return *this;
    }

    template<typename U>
    SharedPtr<T>& operator=(SharedPtr<U>&& Other)
    {
        Move<U>(std::move(Other));
        return *this;
    }

    operator bool() const
    {
        return Get() != nullptr;
    }

    T& operator*()
    {
        LS_ASSERT(m_Data != nullptr);
        return *m_Data;
    }

    T const& operator*() const
    {
        LS_ASSERT(m_Data != nullptr);
        return *m_Data;
    }

    T* operator->()
    {
        LS_ASSERT(m_Data != nullptr);
        return m_Data;
    }

    T const* operator->() const
    {
        LS_ASSERT(m_Data != nullptr);
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

    u32 GetReferenceCount() const
    {
        if (m_ReferenceCount == nullptr)
        {
            return 0;
        }

        return m_ReferenceCount->Count();
    }

    bool IsNull() const
    {
        return m_Data == nullptr;
    }

private:
    friend class WeakPtr<T>;

    template<typename U>
    friend class SharedPtr;

    SharedPtr(T* Data, ReferenceCount* RefCount)
        : m_Data(Data)
        , m_ReferenceCount(RefCount)
    {
        if (m_Data != nullptr)
        {
            m_ReferenceCount->Reference();
        }
    }

    void InitializeReferenceCount()
    {
        if (m_Data == nullptr)
        {
            return;
        }

        if (m_ReferenceCount != nullptr)
        {
            return;
        }

        m_ReferenceCount = new ReferenceCount();
        m_ReferenceCount->Reference();
        m_ReferenceCount->WeakRef();
    }

    void Dereference()
    {
        if (m_ReferenceCount == nullptr)
        {
            return;
        }

        if (m_ReferenceCount->Dereference() == 0)
        {
            delete m_Data;
            m_Data = nullptr;

            m_ReferenceCount->WeakDeref();
        }

        if (m_ReferenceCount->Weaks() == 0)
        {
            delete m_ReferenceCount;
            m_ReferenceCount = nullptr;
        }
    }

    template<typename U>
    void Copy(const SharedPtr<U>& Other)
    {
        if ((void*)this == (void*)&Other)
        {
            return;
        }

        Dereference();

        m_Data = Other.m_Data;
        m_ReferenceCount = Other.m_ReferenceCount;
        m_ReferenceCount->Reference();
    }

    template<typename U>
    void Move(SharedPtr<U>&& Other)
    {
        if ((void*)this == (void*)&Other)
        {
            return;
        }

        Dereference();

        m_Data = std::move(Other.m_Data);
        m_ReferenceCount = std::move(Other.m_ReferenceCount);

        Other.m_Data = nullptr;
        Other.m_ReferenceCount = nullptr;
    }

    T* m_Data { nullptr };
    ReferenceCount* m_ReferenceCount { nullptr };
};

template<typename T, typename U>
static bool operator==(const SharedPtr<T>& A, const SharedPtr<U>& B)
{
    return A.Get() == B.Get();
}

template<typename T>
static bool operator==(const SharedPtr<T>& A, nullptr_t)
{
    return A.Get() == nullptr;
}

template<typename T, typename U>
static bool operator!=(const SharedPtr<T>& A, const SharedPtr<U>& B)
{
    return A.Get() != B.Get();
}

template<typename T>
static bool operator!=(const SharedPtr<T>& A, nullptr_t)
{
    return A.Get() != nullptr;
}

}
}

template<typename T>
using SharedPtr = Core::Memory::SharedPtr<T>;

}

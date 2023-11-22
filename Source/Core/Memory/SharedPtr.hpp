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
#include "../Types.hpp"

#include <utility>

namespace LevelSketch
{
namespace Core
{
namespace Memory
{

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
        InitializeReferenceCount();
    }

    SharedPtr(const SharedPtr<T>& Other)
    {
        Copy(Other);
    }

    SharedPtr(T* Data)
        : m_Data(Data)
    {
        InitializeReferenceCount();
    }

    SharedPtr(SharedPtr<T>&& Other)
    {
        Move(std::move(Other));
    }

    virtual ~SharedPtr()
    {
        Dereference();
    }

    SharedPtr<T>& operator=(const SharedPtr<T>& Other)
    {
        Copy(Other);
        return *this;
    }

    SharedPtr<T>& operator=(SharedPtr<T>&& Other)
    {
        Move(std::move(Other));
        return *this;
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
    class ReferenceCount
    {
    public:
        ReferenceCount()
        {
        }

        void Reference()
        {
            m_Count++;
        }

        u32 Dereference()
        {
            return --m_Count;
        }

        u32 Count() const
        {
            return m_Count;
        }
    
    private:
        u32 m_Count { 0 };
    };

    void InitializeReferenceCount()
    {
        if (m_ReferenceCount != nullptr)
        {
            return;
        }

        m_ReferenceCount = new ReferenceCount();
        m_ReferenceCount->Reference();
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

            delete m_ReferenceCount;
            m_ReferenceCount = nullptr;
        }
    }

    void Copy(const SharedPtr<T>& Other)
    {
        if (this == &Other)
        {
            return;
        }

        Dereference();

        m_Data = Other.m_Data;
        m_ReferenceCount = Other.m_ReferenceCount;
        m_ReferenceCount->Reference();
    }

    void Move(SharedPtr<T>&& Other)
    {
        if (this == &Other)
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

}
}
}

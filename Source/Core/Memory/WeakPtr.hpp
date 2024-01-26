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
#include "SharedPtr.hpp"

namespace LevelSketch
{
namespace Core
{
namespace Memory
{

template<typename T>
class WeakPtr
{
public:
    WeakPtr()
    {
    }

    WeakPtr(const WeakPtr<T>& Other)
        : m_Data(Other.m_Data)
        , m_ReferenceCount(Other.m_ReferenceCount)
    {
        if (m_ReferenceCount != nullptr)
        {
            m_ReferenceCount->WeakRef();
        }
    }

    WeakPtr(const SharedPtr<T>& Other)
        : m_Data(Other.m_Data)
        , m_ReferenceCount(Other.m_ReferenceCount)
    {
        if (m_ReferenceCount != nullptr)
        {
            m_ReferenceCount->WeakRef();
        }
    }

    ~WeakPtr()
    {
        if (m_ReferenceCount != nullptr && m_ReferenceCount->WeakDeref() == 0)
        {
            delete m_ReferenceCount;
            m_ReferenceCount = nullptr;
        }
    }

    WeakPtr<T>& operator=(const WeakPtr<T>& Other)
    {
        if (this == &Other)
        {
            return *this;
        }

        if (m_ReferenceCount != nullptr && m_ReferenceCount->WeakDeref() == 0)
        {
            delete m_ReferenceCount;
            m_ReferenceCount = nullptr;
        }

        m_Data = Other.m_Data;
        m_ReferenceCount = Other.m_ReferenceCount;

        if (m_ReferenceCount != nullptr)
        {
            m_ReferenceCount->WeakRef();
        }

        return *this;
    }

    bool IsValid() const
    {
        if (m_ReferenceCount == nullptr)
        {
            return false;
        }

        if (m_ReferenceCount->Count() == 0)
        {
            return false;
        }

        return true;
    }

    u32 GetReferenceCount() const
    {
        if (m_ReferenceCount != nullptr)
        {
            return m_ReferenceCount->Count();
        }

        return 0;
    }

    SharedPtr<T> Lock() const
    {
        if (IsValid())
        {
            return SharedPtr<T>(m_Data, m_ReferenceCount);
        }

        return SharedPtr<T>(nullptr);
    }

private:
    friend class SharedPtr<T>;

    T* m_Data { nullptr };
    ReferenceCount* m_ReferenceCount { nullptr };
};

}
}

template<typename T>
using WeakPtr = Core::Memory::WeakPtr<T>;

}

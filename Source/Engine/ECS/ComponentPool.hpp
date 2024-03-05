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

#include "../../Core/Containers/Array.hpp"

namespace LevelSketch
{
namespace Engine
{
namespace ECS
{

class ComponentPool
{
public:
    class ConstIterator
    {
    public:
        bool operator==(const ConstIterator& Other) const
        {
            return m_Index == Other.m_Index;
        }

        bool operator!=(const ConstIterator& Other) const
        {
            return m_Index != Other.m_Index;
        }

        ConstIterator& operator++()
        {
            m_Index++;
            return *this;
        }

        template<typename T>
        const T& Get() const
        {
            u8 const* Element { m_Pool.GetElement(m_Index) };
            return *reinterpret_cast<T const*>(Element);
        }

        u64 Index() const
        {
            return m_Index;
        }

    private:
        friend ComponentPool;

        ConstIterator(const ComponentPool& Pool, u64 Index)
            : m_Pool(Pool)
            , m_Index(Index)
        {
        }

        const ComponentPool& m_Pool;
        u64 m_Index { 0 };
    };

    ComponentPool();

    template<typename T>
    T& Get(u64 Index)
    {
        return *reinterpret_cast<T*>(GetElement(Index));
    }

    template<typename T>
    const T& Get(u64 Index) const
    {
        return *reinterpret_cast<T const*>(GetElement(Index));
    }

    ComponentPool& SetElementSize(u64 ElementSize);
    ComponentPool& AddElement();
    u8* GetElement(u64 Index);
    u8 const* GetElement(u64 Index) const;
    u64 Size() const;

    ConstIterator Begin() const;
    ConstIterator End() const;

private:
    friend ConstIterator;

    Array<u8> m_Pool {};
    u64 m_ElementSize { 0 };
};

}
}
}

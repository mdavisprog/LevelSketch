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
#include "../Types.hpp"

namespace LevelSketch
{
namespace Core
{
namespace Memory
{

class ReferenceCount
{
public:
    ReferenceCount()
    {
    }

    ReferenceCount& Reference()
    {
        m_Count++;
        return *this;
    }

    u32 Dereference()
    {
        return --m_Count;
    }

    u32 Count() const
    {
        return m_Count;
    }

    ReferenceCount& WeakRef()
    {
        m_Weaks++;
        return *this;
    }

    u32 WeakDeref()
    {
        return --m_Weaks;
    }

    u32 Weaks() const
    {
        return m_Weaks;
    }

private:
    u32 m_Count { 0 };
    u32 m_Weaks { 0 };
};

}
}
}

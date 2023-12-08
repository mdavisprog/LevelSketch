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

#include "WeakPtr.hpp"

namespace LevelSketch
{
namespace Core
{
namespace Memory
{

template<typename T>
class Shareable
{
public:
    SharedPtr<T> Share() const
    {
        return SharedPtr<T>(m_Weak);
    }

protected:
    Shareable()
    {
    }

    Shareable(const Shareable<T>&)
    {
    }

    Shareable& operator=(const Shareable<T>&)
    {
        return *this;
    }

private:
    friend class SharedPtr<T>;

    WeakPtr<T> m_Weak {};
};

}
}

template<typename T>
using Shareable = Core::Memory::Shareable<T>;

}

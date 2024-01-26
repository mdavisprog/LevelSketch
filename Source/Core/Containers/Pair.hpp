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

namespace LevelSketch
{
namespace Core
{
namespace Containers
{

template<typename _First, typename _Second>
struct Pair
{
    Pair() = default;
    Pair(const Pair&) = default;
    Pair(Pair&&) = default;
    Pair& operator=(const Pair&) = default;

    Pair(const _First& First, const _Second& Second)
        : First(First)
        , Second(Second)
    {
    }

    _First First { 0 };
    _Second Second { 0 };
};

}
}

template<typename _First, typename _Second>
using Pair = Core::Containers::Pair<_First, _Second>;

template<typename _First, typename _Second>
static constexpr Pair<_First, _Second> MakePair(const _First& First, const _Second& Second)
{
    return Pair<_First, _Second> { First, Second };
}

}

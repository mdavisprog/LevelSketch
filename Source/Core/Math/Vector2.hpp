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

#include "../Types.hpp"

namespace LevelSketch
{
namespace Core
{
namespace Math
{

template<typename T>
struct Vector2
{
public:
    T X { 0 };
    T Y { 0 };
};

template<typename T>
static inline Vector2<T> operator+(const Vector2<T>& A, const Vector2<T>& B)
{
    return { A.X + B.X, A.Y + B.Y };
}

template<typename T>
static inline Vector2<T> operator-(const Vector2<T>& A, const Vector2<T>& B)
{
    return { A.X - B.X, A.Y - B.Y };
}

template<typename T>
static inline Vector2<T> operator*(const Vector2<T>& A, const Vector2<T>& B)
{
    return { A.X * B.X, A.Y * B.Y };
}

typedef Vector2<i32> Vector2i;
typedef Vector2<f32> Vector2f;

}
}

using Vector2 = Core::Math::Vector2f;
using Vector2i = Core::Math::Vector2i;

}

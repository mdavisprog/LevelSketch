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
struct Color;

typedef Color<u8> Colorb;
typedef Color<f32> Colorf;

template<typename T>
struct Rect;

typedef Rect<f32> Rectf;
typedef Rect<i32> Recti;

template<typename T>
struct Matrix4;

template<typename T>
struct Vector2;

typedef Vector2<f32> Vector2f;
typedef Vector2<i32> Vector2i;

template<typename T>
struct Vector3;

typedef Vector3<f32> Vector3f;

template<typename T>
struct Vertex3;

}
}

using Color = Core::Math::Colorb;
using Colorf = Core::Math::Colorf;
using Rectf = Core::Math::Rectf;
using Recti = Core::Math::Recti;
using Matrix4f = Core::Math::Matrix4<f32>;
using Vector2 = Core::Math::Vector2f;
using Vector2i = Core::Math::Vector2i;
using Vector3 = Core::Math::Vector3f;
using Vertex3 = Core::Math::Vertex3<f32>;

}

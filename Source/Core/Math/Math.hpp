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

#include "../Compiler.hpp"
#include "../Types.hpp"

#include <cmath>

namespace LevelSketch
{
namespace Core
{
namespace Math
{

static constexpr const f32 PI { 3.14159265359f };
static constexpr const f32 DEG2RAD { PI / 180.0f };
static constexpr const f32 RAD2DEG { 180.0f / PI };

static i32 Abs(i32 Value)
{
    return Value < 0 ? Value * -1 : Value;
}

static f32 Absf(f32 Value)
{
    return Value < 0.0f ? Value * -1.0f : Value;
}

MAYBE_UNUSED static bool IsNearlyEqual(f32 A, f32 B, f32 Precision = 1e-5)
{
    return Absf(A - B) <= Precision;
}

template<typename T>
static T Min(const T& A, const T& B)
{
    return A < B ? A : B;
}

template<typename T>
static T Max(const T& A, const T& B)
{
    return A > B ? A : B;
} 

template<typename T>
static T Sin(T Radians)
{
    return std::sin(Radians);
}

template<typename T>
static T Cos(T Radians)
{
    return std::cos(Radians);
}

template<typename T>
static T FMod(T X, T Y)
{
    return std::fmod(X, Y);
}

}
}

static const auto& PI = Core::Math::PI;
static const auto& DEG2RAD = Core::Math::DEG2RAD;
static const auto& RAD2DEG = Core::Math::RAD2DEG;
static const auto& Abs = Core::Math::Abs;
static const auto& Absf = Core::Math::Absf;
template<typename T> static const auto& Min = Core::Math::Min<T>;
template<typename T> static const auto& Max = Core::Math::Max<T>;
template<typename T> static const auto& Sin = Core::Math::Sin<T>;
template<typename T> static const auto& Cos = Core::Math::Cos<T>;
template<typename T> static const auto& FMod = Core::Math::FMod<T>;

}

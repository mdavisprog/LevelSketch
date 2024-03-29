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
#include "Math.hpp"

namespace LevelSketch
{
namespace Core
{
namespace Math
{

template<typename T>
struct Vector3
{
public:
    static const Vector3<T> Zero;
    static const Vector3<T> Up;
    static const Vector3<T> Right;
    static const Vector3<T> Forward;

    T X { 0 };
    T Y { 0 };
    T Z { 0 };

    bool operator==(const Vector3<T>& Other) const
    {
        return IsNearlyEqual(X, Other.X) && IsNearlyEqual(Y, Other.Y) && IsNearlyEqual(Z, Other.Z);
    }

    Vector3<T>& operator+=(const Vector3<T>& Other)
    {
        X += Other.X;
        Y += Other.Y;
        Z += Other.Z;
        return *this;
    }

    Vector3<T>& operator-=(const Vector3<T>& Other)
    {
        X -= Other.X;
        Y -= Other.Y;
        Z -= Other.Z;
        return *this;
    }

    Vector3<T>& operator*=(const Vector3<T>& Other)
    {
        X *= Other.X;
        Y *= Other.Y;
        Z *= Other.Z;
        return *this;
    }

    Vector3<T>& operator*=(T Scalar)
    {
        X *= Scalar;
        Y *= Scalar;
        Z *= Scalar;
        return *this;
    }

    T Length() const
    {
        return Sqrt(X * X + Y * Y + Z * Z);
    }

    T LengthSq() const
    {
        return X * X + Y * Y + Z * Z;
    }

    Vector3<T> Normalize() const
    {
        T Length { this->Length() };
        if (Length == 0)
        {
            Length = 1;
        }

        return { X / Length, Y / Length, Z / Length };
    }

    Vector3<T> Cross(const Vector3<T>& Other) const
    {
        return { Y * Other.Z - Z * Other.Y, Z * Other.X - X * Other.Z, X * Other.Y - Y * Other.X };
    }

    T Dot(const Vector3<T>& Other) const
    {
        return { X * Other.X + Y * Other.Y + Z * Other.Z };
    }

    Vector3<T>& Clamp(T Length)
    {
        T Current { X * X + Y * Y + Z * Z };

        if (Current > 0)
        {
            Current = Sqrt<T>(Current);

            if (Current > Length)
            {
                const T Scale { Length / Current };
                X *= Scale;
                Y *= Scale;
                Z *= Scale;
            }
        }

        return *this;
    }
};

template<typename T>
const Vector3<T> Vector3<T>::Zero { 0, 0, 0 };

template<typename T>
const Vector3<T> Vector3<T>::Up { 0, 1, 0 };

template<typename T>
const Vector3<T> Vector3<T>::Right { 1, 0, 0 };

template<typename T>
const Vector3<T> Vector3<T>::Forward { 0, 0, 1 };

typedef Vector3<f32> Vector3f;
typedef Vector3<f64> Vector3d; // double

template<typename T>
static inline Vector3<T> operator+(const Vector3<T>& A, const Vector3<T>& B)
{
    return { A.X + B.X, A.Y + B.Y, A.Z + B.Z };
}

template<typename T>
static inline Vector3<T> operator-(const Vector3<T>& A, const Vector3<T>& B)
{
    return { A.X - B.X, A.Y - B.Y, A.Z - B.Z };
}

template<typename T>
static inline Vector3<T> operator*(const Vector3<T>& A, const Vector3<T>& B)
{
    return { A.X * B.X, A.Y * B.Y, A.Z * B.Z };
}

template<typename T>
static inline Vector3<T> operator*(const Vector3<T>& A, T Scalar)
{
    return { A.X * Scalar, A.Y * Scalar, A.Z * Scalar };
}

}
}

using Vector3 = Core::Math::Vector3f;

}

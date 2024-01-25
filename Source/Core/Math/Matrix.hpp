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

#include "../Types.hpp"
#include "Math.hpp"
#include "Rect.hpp"
#include "Vector3.hpp"

#include <initializer_list>

namespace LevelSketch
{
namespace Core
{
namespace Math
{

// ROW-MAJOR
template<typename T>
struct Matrix4
{
public:
    static Matrix4<T> Identity;
    static Matrix4<T> Zero;

    static Matrix4<T> LookAt(const Vector3<T>& Eye, const Vector3<T>& Center, const Vector3<T>& Up)
    {
        const Vector3<T> Forward { (Eye - Center).Normalize() };
        const Vector3<T> Side { Forward.Cross(Up).Normalize() };
        const Vector3<T> dUp { Side.Cross(Forward) };

        return
        {
            Side.X, Side.Y, Side.Z, 0,
            dUp.X, dUp.Y, dUp.Z, 0,
            Forward.X, Forward.Y, Forward.Z, 0,
            Eye.X, Eye.Y, Eye.Z, 1
        };
    }

    T Data[16];

    Matrix4() = default;
    Matrix4(const Matrix4<T>&) = default;
    Matrix4<T>& operator=(const Matrix4<T>&) = default;

    Matrix4(T InData[16])
        : Data(InData)
    {
    }

    Matrix4(const std::initializer_list<T>& List)
    {
        for (std::size_t I = 0; I < List.size(); I++)
        {
            Data[I] = *(List.begin() + I);
        }
    }

    T& operator[](i32 Index)
    {
        return Data[Index];
    }

    const T& operator[](i32 Index) const
    {
        return Data[Index];
    }

    bool operator==(const Matrix4& Other) const
    {
        for (i32 I = 0; I < 16; I++)
        {
            if (Data[I] != Other[I])
            {
                return false;
            }
        }

        return true;
    }

    Matrix4<T> Transpose() const
    {
        return
        {
            Data[0], Data[4], Data[8], Data[12],
            Data[1], Data[5], Data[9], Data[13],
            Data[2], Data[6], Data[10], Data[14],
            Data[3], Data[7], Data[11], Data[15]
        };
    }

    Matrix4<T> RotateX(T Angle) const
    {
        const T CosRad { Cos<T>(Angle * DEG2RAD) };
        const T SinRad { Sin<T>(Angle * DEG2RAD) };

        Matrix4<T> Rotation
        {
            1, 0, 0, 0,
            0, CosRad, -SinRad, 0,
            0, SinRad, CosRad, 0,
            0, 0, 0, 1
        };

        return *this * Rotation;
    }

    Matrix4<T> RotateY(T Angle) const
    {
        const T CosRad { Cos<T>(Angle * DEG2RAD) };
        const T SinRad { Sin<T>(Angle * DEG2RAD) };

        Matrix4<T> Rotation
        {
            CosRad, 0, SinRad, 0,
            0, 1, 0, 0,
            -SinRad, 0, CosRad, 0,
            0, 0, 0, 1
        };

        return *this * Rotation;
    }

    Matrix4<T> RotateZ(T Angle) const
    {
        const T CosRad { Cos<T>(Angle * DEG2RAD) };
        const T SinRad { Sin<T>(Angle * DEG2RAD) };

        Matrix4<T> Rotation
        {
            CosRad, -SinRad, 0, 0,
            SinRad, CosRad, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };

        return *this * Rotation;
    }
};

template<typename T>
Matrix4<T> Matrix4<T>::Identity
{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

template<typename T>
Matrix4<T> Matrix4<T>::Zero { 0 };

typedef Matrix4<f32> Matrix4f;

template<typename T>
static inline Matrix4<T> operator*(const Matrix4<T>& A, T Scalar)
{
    Matrix4<T> Result {};

    for (u32 I = 0; I < 16; I++)
    {
        Result[I] = A[I] * Scalar;
    }

    return Result;
}

template<typename T>
static inline Matrix4<T> operator*(T Scalar, const Matrix4<T>& A)
{
    Matrix4<T> Result {};

    for (u32 I = 0; I < 16; I++)
    {
        Result[I] = Scalar * A[I];
    }

    return Result;
}

template<typename T>
static inline Matrix4<T> operator*(const Matrix4<T>& A, const Matrix4<T>& B)
{
    Matrix4<T> Result;

    for (u32 Row = 0; Row < 4; Row++)
    {
        for (u32 Col = 0; Col < 4; Col++)
        {
            const u32 Destination { Row * 4 + Col };
            Result[Destination] = 0.0f;

            for (u32 I = 0; I < 4; I++)
            {
                const u32 RowIndex { Row * 4 + I };
                const u32 ColIndex { Col + I * 4 };
                Result[Destination] += A[RowIndex] * B[ColIndex];
            }
        }
    }

    return Result;
}

static inline Matrix4f PerspectiveMatrixRH(f32 FOVAngle, f32 Aspect, f32 Near, f32 Far)
{
    const f32 FOVRad { FOVAngle * DEG2RAD };
    const f32 YScale { 1.0f / Tan<f32>(FOVRad * 0.5f) };
    const float XScale { YScale * Aspect };
    return
    {
        XScale, 0.0f, 0.0f, 0.0f,
        0.0f, YScale, 0.0f, 0.0,
        0.0f, 0.0f, Far / (Far - Near), 1.0f,
        0.0f, 0.0f, -Near * (Far / (Far - Near)), 0.0f
    };
}

static inline Matrix4f OrthographicMatrixRH(Rectf Viewport, f32 Near, f32 Far)
{
    const f32 L { Viewport.Left() };
    const f32 R { Viewport.Right() };
    const f32 T { Viewport.Top() };
    const f32 B { Viewport.Bottom() };
    const f32 ReciprocalWidth { 1.0f / (R - L) };
    const f32 ReciprocalHeight { 1.0f / (T - B) };
    const f32 Range { 1.0f / (Near - Far) };
    return
    {
        2.0f * ReciprocalWidth, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f * ReciprocalHeight, 0.0f, 0.0f,
        0.0f, 0.0f, Range, 0.0f,
        -(L + R) * ReciprocalWidth, -(T + B) * ReciprocalHeight, Range * Near, 1.0f
    };
}

}
}

using Matrix4f = Core::Math::Matrix4f;

}

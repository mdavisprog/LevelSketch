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
#include "Rect.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"

#include <initializer_list>

namespace LevelSketch
{
namespace Core
{
namespace Math
{
// clang-format off

// COLUMN-MAJOR
template<typename T>
struct Matrix4
{
public:
    static Matrix4<T> Identity;
    static Matrix4<T> Zero;

    static Matrix4<T> Translation(const Vector3<T>& Translate)
    {
        return
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            Translate.X, Translate.Y, Translate.Z, 1
        };
    }

    static Matrix4<T> Scale(T Scalar)
    {
        return
        {
            Scalar, 0, 0, 0,
            0, Scalar, 0, 0,
            0, 0, Scalar, 0,
            0, 0, 0, 1
        };
    }

    static Matrix4<T> Scale(const Vector3<T>& Scale)
    {
        return
        {
            Scale.X, 0, 0, 0,
            0, Scale.Y, 0, 0,
            0, 0, Scale.Z, 0,
            0, 0, 0, 1
        };
    }

    static Matrix4<T> RotationX(T Angle)
    {
        const T Theta { Angle * DEG2RAD };
        const T Cosine { Cos<T>(Theta) };
        const T Sine { Sin<T>(Theta) };
        
        return
        {
            1, 0, 0, 0,
            0, Cosine, -Sine, 0,
            0, Sine, Cosine, 0,
            0, 0, 0, 1
        };
    }

    static Matrix4<T> RotationY(T Angle)
    {
        const T Theta { Angle * DEG2RAD };
        const T Cosine { Cos<T>(Theta) };
        const T Sine { Sin<T>(Theta) };
        
        return
        {
            Cosine, 0, Sine, 0,
            0, 1, 0, 0,
            -Sine, 0, Cosine, 0,
            0, 0, 0, 1
        };
    }

    static Matrix4<T> RotationZ(T Angle)
    {
        const T Theta { Angle * DEG2RAD };
        const T Cosine { Cos<T>(Theta) };
        const T Sine { Sin<T>(Theta) };
        
        return
        {
            Cosine, -Sine, 0, 0,
            Sine, Cosine, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
    }

    static Matrix4<T> LookAtLH(const Vector3<T>& Eye, const Vector3<T>& Center, const Vector3<T>& Up)
    {
        const Vector3<T> Forward { (Center - Eye).Normalize() };
        const Vector3<T> Side { Up.Cross(Forward).Normalize() };
        const Vector3<T> dUp { Forward.Cross(Side) };

        return
        {
            Side.X, dUp.X, Forward.X, 0,
            Side.Y, dUp.Y, Forward.Y, 0,
            Side.Z, dUp.Z, Forward.Z, 0,
            -(Eye.Dot(Side)), -(Eye.Dot(dUp)), -(Eye.Dot(Forward)), 1
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

    Vector4<T> Row(u32 Index) const
    {
        return { Data[Index * 4], Data[Index * 4 + 1], Data[Index * 4 + 2], Data[Index * 4 + 3] };
    }

    Vector4<T> Column(u32 Index) const
    {
        return { Data[Index], Data[Index + (1 * 4)], Data[Index + (2 * 4)], Data[Index + (3 * 4)] };
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

template<typename T>
static inline Matrix4<T> operator+(const Matrix4<T>& A, const Matrix4<T>& B)
{
    Matrix4<T> Result;

    for (u32 I = 0; I < 16; I++)
    {
        Result[I] = A[I] + B[I];
    }

    return Result;
}

template<typename T>
static inline Matrix4<T> operator-(const Matrix4<T>& A, const Matrix4<T>& B)
{
    Matrix4<T> Result;

    for (u32 I = 0; I < 16; I++)
    {
        Result[I] = A[I] - B[I];
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

template<typename T>
static inline Vector3<T> operator*(const Matrix4<T>& A, const Vector3<T>& Vec)
{
    return
    {
        A[0] * Vec.X + A[1] * Vec.Y + A[2] * Vec.Z + A[3],
        A[4] * Vec.X + A[5] * Vec.Y + A[6] * Vec.Z + A[7],
        A[8] * Vec.X + A[9] * Vec.Y + A[10] * Vec.Z + A[11]
    };
}

template<typename T>
static inline Vector4<T> operator*(const Matrix4<T>& A, const Vector4<T>& Vec)
{
    return
    {
        A[0] * Vec.X + A[1] * Vec.Y + A[2] * Vec.Z + A[3] * Vec.W,
        A[4] * Vec.X + A[5] * Vec.Y + A[6] * Vec.Z + A[7] * Vec.W,
        A[8] * Vec.X + A[9] * Vec.Y + A[10] * Vec.Z + A[11] * Vec.W,
        A[12] * Vec.X + A[13] * Vec.Y + A[14] * Vec.Z + A[15] * Vec.W
    };
}

typedef Matrix4<f32> Matrix4f;

// Uses clip space depth of [0, 1]. This seems to be the standard across DirectX, Metal, and Vulkan.
static inline Matrix4f PerspectiveMatrixLH(f32 FOVAngle, f32 Aspect, f32 Near, f32 Far)
{
    const f32 FOVRad { FOVAngle * DEG2RAD };
    const f32 YScale { 1.0f / Tan<f32>(FOVRad * 0.5f) };
    const float XScale { YScale / Aspect };
    return
    {
        XScale, 0.0f, 0.0f, 0.0f,
        0.0f, YScale, 0.0f, 0.0,
        0.0f, 0.0f, Far / (Far - Near), 1.0f,
        0.0f, 0.0f, (-Near * Far) / (Far - Near), 0.0f
    };
}

static inline Matrix4f OrthographicMatrixLH(Rectf Viewport, f32 Near, f32 Far)
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

// clang-format on
}
}

using Matrix4f = Core::Math::Matrix4f;

}

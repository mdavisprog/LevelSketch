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

}
}

using Matrix4f = Core::Math::Matrix4f;

}

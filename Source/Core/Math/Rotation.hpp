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

#include "Matrix.hpp"

#include <initializer_list>

namespace LevelSketch
{
namespace Core
{
namespace Math
{

class Rotation
{
public:
    Rotation()
    {
    }

    Rotation(f32 InPitch, f32 InYaw, f32 InRoll)
        : m_Pitch(InPitch)
        , m_Yaw(InYaw)
        , m_Roll(InRoll)
    {
    }

    Rotation(const std::initializer_list<f32>& List)
    {
        m_Pitch = List.begin()[0];
        m_Yaw = List.begin()[1];
        m_Roll = List.begin()[2];
    }

    Rotation& SetPitch(f32 Pitch)
    {
        m_Pitch = FMod<f32>(Pitch, 360.0f);
        return *this;
    }

    Rotation& AddPitch(f32 Pitch)
    {
        m_Pitch = FMod<f32>(m_Pitch + Pitch, 360.0f);
        return *this;
    }

    f32 Pitch() const
    {
        return m_Pitch;
    }

    Rotation& SetYaw(f32 Yaw)
    {
        m_Yaw = FMod<f32>(Yaw, 360.0f);
        return *this;
    }

    Rotation& AddYaw(f32 Yaw)
    {
        m_Yaw = FMod<f32>(m_Yaw + Yaw, 360.0f);
        return *this;
    }

    f32 Yaw() const
    {
        return m_Yaw;
    }

    Rotation& SetRoll(f32 Roll)
    {
        m_Roll = Roll;
        return *this;
    }

    Rotation& AddRoll(f32 Roll)
    {
        m_Roll = FMod<f32>(m_Roll + Roll, 360.0f);
        return *this;
    }

    f32 Roll() const
    {
        return m_Roll;
    }

    Matrix4f ToMatrix() const
    {
        return Matrix4f::RotationY(m_Yaw) * Matrix4f::RotationX(-m_Pitch) * Matrix4f::RotationZ(m_Roll);
    }

private:
    f32 m_Pitch { 0.0f };
    f32 m_Yaw { 0.0f };
    f32 m_Roll { 0.0f };
};

}
}

using Rotation = Core::Math::Rotation;

}

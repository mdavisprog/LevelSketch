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

#include "Camera.hpp"

namespace LevelSketch
{
namespace Engine
{

Camera::Camera()
{
}

Camera::Camera(const Vector3& Position)
    : m_Position(Position)
{
}

Camera& Camera::SetPosition(const Vector3& Position)
{
    m_Position = Position;
    return *this;
}

Vector3 Camera::Position() const
{
    return m_Position;
}

Camera& Camera::SetMovement(u8 Flags)
{
    m_Movement |= Flags;
    return *this;
}

Camera& Camera::ClearMovement(u8 Flags)
{
    m_Movement &= ~Flags;
    return *this;
}

Camera& Camera::SetSpeed(f32 Speed)
{
    m_Speed = Speed;
    return *this;
}

Matrix4f Camera::ToViewMatrix() const
{
    return Matrix4f::LookAtLH(m_Position, m_Position + m_Direction, Vector3::Up);
}

Camera& Camera::Update(float DeltaTime)
{
    if (m_Movement & Movement::Left)
    {
        m_Velocity.X += -m_Speed;
    }

    if (m_Movement & Movement::Right)
    {
        m_Velocity.X += m_Speed;
    }

    if (m_Movement & Movement::Backward)
    {
        m_Velocity.Z += -m_Speed;
    }

    if (m_Movement & Movement::Forward)
    {
        m_Velocity.Z += m_Speed;
    }

    m_Velocity.Clamp(m_MaxSpeed);
    m_Position += m_Velocity * DeltaTime;
    m_Velocity *= 0.9f;

    return *this;
}

}
}

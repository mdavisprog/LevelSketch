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

#include "../Core/Math/Matrix.hpp"

namespace LevelSketch
{
namespace Engine
{

class Camera
{
public:
    struct Movement
    {
        enum Flags : u8
        {
            None = 0,
            Forward = 1 << 0,
            Backward = 1 << 1,
            Left = 1 << 2,
            Right = 1 << 3
        };
    };

    Camera();
    Camera(const Vector3& Position);

    Camera& SetPosition(const Vector3& Position);
    Vector3 Position() const;

    Camera& SetMovement(u8 Flags);
    Camera& ClearMovement(u8 Flags);

    Camera& SetSpeed(f32 Speed);

    Matrix4f ToViewMatrix() const;

    Camera& Update(float DeltaTime);

private:
    Vector3 m_Position {};
    Vector3 m_Direction { Vector3::Forward };
    Vector3 m_Velocity {};
    u8 m_Movement { Movement::None };
    f32 m_Speed { 2.0f };
    f32 m_MaxSpeed { 20.0f };
};

}
}

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

#include "../../Core/Math/Math.hpp"
#include "../../Core/Math/Color.hpp"
#include "../../Core/Math/Matrix.hpp"
#include "../../Core/Math/Rotation.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "../../Core/Math/Vector3.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"
#include "Core.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

static bool CommonOps()
{
    VERIFY(Abs(-5) == 5);
    VERIFY(Absf(1.0f) == 1.0f);
    VERIFY(Min<i32>(3, 5) == 3);
    VERIFY(Min<i32>({ 1, 3, 5, 7 }) == 1);
    VERIFY(Max<i32>(3, 5) == 5);
    VERIFY(Max<i32>({ 1, 3, 5, 7 }) == 7);

    constexpr const f32 Angle { 90.0f * DEG2RAD };
    VERIFY(Sin<f32>(Angle) == 1.0f);
    VERIFY(LevelSketch::Core::Math::IsNearlyEqual(Cos<f32>(Angle), 0.0f));
    VERIFY(FMod<f32>(4.0f, 4.0f) == 0.0f);
    return true;
}

static bool Vector2Ops()
{
    Vector2i Vector { 3, 5 };
    VERIFY(Vector.X == 3);
    VERIFY(Vector.Y == 5);
    return true;
}

static bool Vector3Ops()
{
    Vector3 Vector { 3.0f, 5.0f, 7.0f };
    VERIFY(Vector.X == 3.0f);
    VERIFY(Vector.Y == 5.0f);
    VERIFY(Vector.Z == 7.0f);
    return true;
}

static bool ColorOps()
{
    Color Colorb { 255, 0, 0, 255 };
    VERIFY(Colorb.R == 255);
    VERIFY(Colorb.G == 0);
    VERIFY(Colorb.B == 0);
    VERIFY(Colorb.A == 255);

    Colorf Colorf { 0.0f, 1.0f, 0.0f, 1.0f };
    VERIFY(Colorf.R == 0.0f);
    VERIFY(Colorf.G == 1.0f);
    VERIFY(Colorf.B == 0.0f);
    VERIFY(Colorf.A == 1.0f);

    return true;
}

// clang-format off
namespace Matrix
{

static const Matrix4f Ascending
{
    1.0f, 2.0f, 3.0f, 4.0f,
    5.0f, 6.0f, 7.0f, 8.0f,
    9.0f, 10.0f, 11.0f, 12.0f,
    13.0f, 14.0f, 15.0f, 16.0f
};

static const Matrix4f Descending
{
    16.0f, 15.0f, 14.0f, 13.0f,
    12.0f, 11.0f, 10.0f, 9.0f,
    8.0f, 7.0f, 6.0f, 5.0f,
    4.0f, 3.0f, 2.0f, 1.0f
};

static bool Identity()
{
    const Matrix4f A
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    VERIFY(A == Matrix4f::Identity);

    return true;
}

static bool RowColumnAccess()
{
    VERIFY(Ascending.Row(0) == (Vector4{1.0f, 2.0f, 3.0f, 4.0f}));
    VERIFY(Ascending.Row(1) == (Vector4{5.0f, 6.0f, 7.0f, 8.0f}));
    VERIFY(Ascending.Row(2) == (Vector4{9.0f, 10.0f, 11.0f, 12.0f}));
    VERIFY(Ascending.Row(3) == (Vector4{13.0f, 14.0f, 15.0f, 16.0f}));
    VERIFY(Ascending.Column(0) == (Vector4{1.0f, 5.0f, 9.0f, 13.0f}));
    VERIFY(Ascending.Column(1) == (Vector4{2.0f, 6.0f, 10.0f, 14.0f}));
    VERIFY(Ascending.Column(2) == (Vector4{3.0f, 7.0f, 11.0f, 15.0f}));
    VERIFY(Ascending.Column(3) == (Vector4{4.0f, 8.0f, 12.0f, 16.0f}));
    return true;
}

static bool Add()
{
    VERIFY(Ascending + Ascending == (Matrix4f
    {
        2.0f, 4.0f, 6.0f, 8.0f,
        10.0f, 12.0f, 14.0f, 16.0f,
        18.0f, 20.0f, 22.0f, 24.0f,
        26.0f, 28.0f, 30.0f, 32.0f
    }));
    return true;
}

static bool Subtract()
{
    VERIFY(Ascending - Descending == (Matrix4f
    {
        -15.0f, -13.0f, -11.0f, -9.0f,
        -7.0f, -5.0f, -3.0f, -1.0f,
        1.0f, 3.0f, 5.0f, 7.0f,
        9.0f, 11.0f, 13.0f, 15.0f
    }))
    return true;
}

static bool Multiply()
{
    VERIFY(Ascending * Descending == (Matrix4f
    {
        80.0f, 70.0f, 60.0f, 50.0f,
        240.0f, 214.0f, 188.0f, 162.0f,
        400.0f, 358.0f, 316.0f, 274.0f,
        560.0f, 502.0f, 444.0f, 386.0f
    }));
    VERIFY(Descending * Ascending == (Matrix4f
    {
        386.0f, 444.0f, 502.0f, 560.0f,
        274.0f, 316.0f, 358.0f, 400.0f,
        162.0f, 188.0f, 214.0f, 240.0f,
        50.0f, 60.0f, 70.0f, 80.0f
    }));
    return true;
}

static bool Transpose()
{
    VERIFY(Ascending.Transpose() == (Matrix4f
    {
        1.0f, 5.0f, 9.0f, 13.0f,
        2.0f, 6.0f, 10.0f, 14.0f,
        3.0f, 7.0f, 11.0f, 15.0f,
        4.0f, 8.0f, 12.0f, 16.0f
    }));
    return true;
}

static bool Translation()
{
    const Matrix4f A { Matrix4f::Translation({1.0f, 2.0f, 3.0f}) };
    VERIFY(A * (Vector4{1.0f, 2.0f, 3.0f, 1.0f}) == (Vector4{1.0f, 2.0f, 3.0f, 15.0}));
    return true;
}

static bool Scale()
{
    const Matrix4f A { Matrix4f::Scale(2.0f) };
    VERIFY(A * (Vector3{1.0f, 2.0f, 3.0f}) == (Vector3{2.0f, 4.0f, 6.0f}));
    return true;
}

}
// clang-format on

static bool RotationToVector()
{
    VERIFY((Rotation {}.ToMatrix() * Vector3::Forward) == Vector3::Forward);
    VERIFY((Rotation { 0.0f, 90.0f, 0.0f }.ToMatrix() * Vector3::Forward) == Vector3::Right);
    VERIFY((Rotation { 90.0f, 0.0f, 0.0f }.ToMatrix() * Vector3::Forward) == Vector3::Up);
    return true;
}

UniquePtr<TestSuite> MathTests()
{
    return TestSuite::New("Math",
        { TEST_CASE(CommonOps),
            TEST_CASE(Vector2Ops),
            TEST_CASE(Vector3Ops),
            TEST_CASE(ColorOps),
            TEST_CASE(Matrix::Identity),
            TEST_CASE(Matrix::RowColumnAccess),
            TEST_CASE(Matrix::Add),
            TEST_CASE(Matrix::Subtract),
            TEST_CASE(Matrix::Multiply),
            TEST_CASE(Matrix::Transpose),
            TEST_CASE(Matrix::Translation),
            TEST_CASE(Matrix::Scale),
            TEST_CASE(RotationToVector) });
}

}
}
}

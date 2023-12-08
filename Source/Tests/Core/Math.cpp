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

#include "Core.hpp"
#include "../../Core/Math/Color.hpp"
#include "../../Core/Math/Math.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "../../Core/Math/Vector3.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

static bool CommonOps()
{
    VERIFY(Min<i32>(3, 5) == 3);
    VERIFY(Max<i32>(3, 5) == 5);
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

UniquePtr<TestSuite> MathTests()
{
    return TestSuite::New("Math", {
        TEST_CASE(CommonOps),
        TEST_CASE(Vector2Ops),
        TEST_CASE(Vector3Ops),
        TEST_CASE(ColorOps)
    });
}

}
}
}

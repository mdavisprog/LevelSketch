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

#include "../../Core/Containers/String.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"
#include "Core.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

static bool Empty()
{
    String Empty {};
    VERIFY(Empty.IsEmpty());
    return true;
}

static bool Constructor()
{
    String Instance { "Hello" };
    VERIFY(!Instance.IsEmpty());
    VERIFY(Instance.Length() == 5);
    return true;
}

static bool Equality()
{
    String Instance { "Hello" };
    VERIFY(Instance == "Hello");
    VERIFY(Instance != "World");
    return true;
}

static bool Conversions()
{
    String Ascii { "Hello" };
    WString Wide { LevelSketch::Core::Containers::ToWString(Ascii) };
    VERIFY(Wide == L"Hello");
    VERIFY(Wide.Length() == 5);

    Ascii = LevelSketch::Core::Containers::ToString(Wide);
    VERIFY(Ascii == "Hello");
    VERIFY(Ascii.Length() == 5);

    VERIFY(LevelSketch::Core::Containers::ToInt("5") == 5);
    VERIFY(LevelSketch::Core::Containers::ToFloat("3.14f") == 3.14f);

    return true;
}

static bool Reserve()
{
    String Instance;
    // Empty string will have an allocation with the null terminator.
    VERIFY(Instance.Capacity() != 0);
    Instance.Reserve(5);
    VERIFY(Instance.Size() == 0);
    VERIFY(Instance.Capacity() == 5);
    Instance = "Hello";
    VERIFY(Instance.Size() == 5);
    VERIFY(Instance.Capacity() > 5);
    return true;
}

static bool Find()
{
    const String Instance { "Hello World" };
    VERIFY(Instance.Find('H') == 0);
    VERIFY(Instance.Find(' ') == 5);
    VERIFY(Instance.Find('d') == 10);
    VERIFY(Instance.Find('T') == String::NPOS);
    VERIFY(Instance.Find('W', 5) == 6);
    VERIFY(Instance.Find('H', 5) == String::NPOS);
    return true;
}

static bool RFind()
{
    const String Instance { "Hello World" };
    VERIFY(Instance.RFind('H') == 0);
    VERIFY(Instance.RFind(' ') == 5);
    VERIFY(Instance.RFind('d') == 10);
    VERIFY(Instance.RFind('T') == String::NPOS);
    VERIFY(Instance.RFind('W', 5) == String::NPOS);
    VERIFY(Instance.RFind('H', 5) == 0);
    return true;
}

static bool Sub()
{
    const String Instance { "Hello World" };
    VERIFY(Instance.Sub(0, 5) == "Hello");
    VERIFY(Instance.Sub(6) == "World");
    VERIFY(Instance.Sub(12) == "");
    return true;
}

static bool Add()
{
    String A { "Hello" };
    String B { "World" };
    VERIFY((A + B) == "HelloWorld");
    return true;
}

static bool Append()
{
    String A { "Hello" };
    String B { "World" };
    A += B;
    VERIFY(A == "HelloWorld");
    A += "Foo";
    VERIFY(A == "HelloWorldFoo");
    String C {};
    C += 'A';
    VERIFY(C == "A");
    C += 'B';
    VERIFY(C == "AB");
    return true;
}

UniquePtr<TestSuite> StringTests()
{
    return TestSuite::New("String",
        { TEST_CASE(Empty),
            TEST_CASE(Constructor),
            TEST_CASE(Equality),
            TEST_CASE(Conversions),
            TEST_CASE(Reserve),
            TEST_CASE(Find),
            TEST_CASE(RFind),
            TEST_CASE(Sub),
            TEST_CASE(Add),
            TEST_CASE(Append) });
}

}
}
}

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
#include "../../Core/Containers/String.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

using String = LevelSketch::Core::String;

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

Memory::UniquePtr<TestSuite> StringTests()
{
    return TestSuite::New("String", {
        TEST_CASE(Empty),
        TEST_CASE(Constructor),
        TEST_CASE(Equality)
    });
}

}
}
}

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

#include "Core.hpp"
#include "../../Core/Containers/Map.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

#include <map>

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

static bool Empty()
{
    Map<i32, i32> Instance;
    VERIFY(Instance.IsEmpty());
    return true;
}

static bool Index()
{
    Map<i32, i32> Instance;
    Instance[5] = 10;
    VERIFY(Instance.Size() == 1);
    VERIFY(Instance[5] == 10);
    return true;
}

static bool ConstIndex()
{
    const Map<i32, i32> Instance
    {
        MakePair(5, 10),
        MakePair(10, 20)
    };
    VERIFY(Instance.Size() == 2);
    VERIFY(Instance[5] == 10);
    VERIFY(Instance[10] == 20);
    return true;
}

static bool Clear()
{
    Map<i32, i32> Instance;
    Instance[10] = 10;
    Instance[20] = 20;
    VERIFY(Instance.Size() == 2);
    Instance.Clear();
    VERIFY(Instance.Size() == 0);
    VERIFY(Instance.IsEmpty());
    return true;
}

static bool Contains()
{
    Map<i32, i32> Instance;
    Instance[10] = 10;
    Instance[20] = 20;
    VERIFY(Instance.Contains(10));
    VERIFY(Instance.Contains(20));
    VERIFY(!Instance.Contains(30));
    Instance.Clear();
    VERIFY(!Instance.Contains(10));
    VERIFY(!Instance.Contains(20));
    VERIFY(!Instance.Contains(30));
    return true;
}

UniquePtr<TestSuite> MapTests()
{
    return TestSuite::New("Map", {
        TEST_CASE(Empty),
        TEST_CASE(Index),
        TEST_CASE(ConstIndex),
        TEST_CASE(Clear),
        TEST_CASE(Contains)
    });
}

}
}
}

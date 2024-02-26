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

#include "../../Core/Containers/HashSet.hpp"
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
    HashSet<i32> Set;
    VERIFY(Set.IsEmpty());
    return true;
}

static bool Insert()
{
    HashSet<i32> Set;
    Set.Insert(1);
    VERIFY(Set.Size() == 1);
    VERIFY(Set.Contains(1));
    return true;
}

static bool InitializerList()
{
    HashSet<i32> Set { 1, 2, 3 };
    VERIFY(Set.Size() == 3);
    VERIFY(Set.Contains(1));
    VERIFY(Set.Contains(2));
    VERIFY(Set.Contains(3));
    VERIFY(!Set.Contains(0));
    return true;
}

static bool UniqueKeys()
{
    HashSet<i32> Set { 1, 2, 2, 3, 4, 5, 4 };
    VERIFY(Set.Size() == 5);
    VERIFY(Set.Contains(1));
    VERIFY(Set.Contains(2));
    VERIFY(Set.Contains(3));
    VERIFY(Set.Contains(4));
    VERIFY(Set.Contains(5));
    Set.Insert(1);
    VERIFY(Set.Size() == 5);
    VERIFY(Set.Contains(1));
    return true;
}

static bool Clear()
{
    HashSet<i32> Set { 1, 2, 3, 2, 1 };
    VERIFY(Set.Size() == 3);
    Set.Clear();
    VERIFY(Set.Size() == 0);
    VERIFY(Set.IsEmpty());
    return true;
}

static bool Remove()
{
    HashSet<i32> Set { 1, 2, 3, 4, 5 };
    VERIFY(Set.Size() == 5);
    Set.Remove(2);
    VERIFY(Set.Size() == 4);
    VERIFY(!Set.Contains(2));
    return true;
}

UniquePtr<TestSuite> HashSetTests()
{
    return TestSuite::New("HashSet",
        { TEST_CASE(Empty),
            TEST_CASE(Insert),
            TEST_CASE(InitializerList),
            TEST_CASE(UniqueKeys),
            TEST_CASE(Clear),
            TEST_CASE(Remove) });
}

}
}
}

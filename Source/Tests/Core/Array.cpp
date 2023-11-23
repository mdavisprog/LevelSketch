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
#include "../../Core/Containers/Array.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

#include <vector>

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

static bool Empty()
{
    LevelSketch::Core::Containers::Array<int> Values;
    VERIFY(Values.Capacity() == 0);
    VERIFY(Values.Size() == 0);
    VERIFY(Values.Data() == nullptr);
    return true;
}

static bool Copy()
{
    LevelSketch::Core::Containers::Array<int> Values { 1, 2, 3 };
    LevelSketch::Core::Containers::Array<int> ValuesCopy { Values };
    VERIFY(Values.Size() == 3);
    VERIFY(Values[0] == 1);
    VERIFY(Values[1] == 2);
    VERIFY(Values[2] == 3);
    VERIFY(ValuesCopy.Size() == 3);
    VERIFY(ValuesCopy[0] == 1);
    VERIFY(ValuesCopy[1] == 2);
    VERIFY(ValuesCopy[2] == 3);
    VERIFY(Values.Data() != ValuesCopy.Data());
    return true;
}

static bool Move()
{
    LevelSketch::Core::Containers::Array<int> Values { 1, 2, 3 };
    VERIFY(Values.Size() == 3);
    VERIFY(Values[0] == 1);
    VERIFY(Values[1] == 2);
    VERIFY(Values[2] == 3);
    LevelSketch::Core::Containers::Array<int> ValuesMove { std::move(Values) };
    VERIFY(Values.Capacity() == 0);
    VERIFY(Values.Size() == 0);
    VERIFY(Values.Data() == nullptr);
    VERIFY(ValuesMove.Size() == 3);
    VERIFY(ValuesMove[0] == 1);
    VERIFY(ValuesMove[1] == 2);
    VERIFY(ValuesMove[2] == 3);
    return true;
}

static bool InitializerList()
{
    LevelSketch::Core::Containers::Array<int> Values { 1, 2, 3 };
    VERIFY(Values.Size() == 3);
    VERIFY(Values[0] == 1);
    VERIFY(Values[1] == 2);
    VERIFY(Values[2] == 3);
    return true;
}

static bool AssignCopy()
{
    LevelSketch::Core::Containers::Array<int> Values { 1, 2, 3 };
    VERIFY(Values.Size() == 3);
    LevelSketch::Core::Containers::Array<int> ValuesCopy;
    VERIFY(ValuesCopy.Size() == 0);
    ValuesCopy = Values;
    VERIFY(ValuesCopy.Size() == 3 && Values.Size() == 3);
    VERIFY(ValuesCopy.Data() != Values.Data());
    VERIFY(Values[0] == 1 && ValuesCopy[0] == 1);
    VERIFY(Values[1] == 2 && ValuesCopy[1] == 2);
    VERIFY(Values[2] == 3 && ValuesCopy[2] == 3);
    return true;
}

static bool AssignMove()
{
    LevelSketch::Core::Containers::Array<int> Values { 1, 2, 3 };
    VERIFY(Values.Size() == 3);
    LevelSketch::Core::Containers::Array<int> ValuesMove;
    VERIFY(ValuesMove.Size() == 0);
    ValuesMove = std::move(Values);
    VERIFY(ValuesMove.Size() == 3 && Values.Size() == 0);
    VERIFY(ValuesMove.Data() != Values.Data() && Values.Data() == nullptr);
    VERIFY(ValuesMove[0] == 1);
    VERIFY(ValuesMove[1] == 2);
    VERIFY(ValuesMove[2] == 3);
    return true;
}

static bool Subscript()
{
    LevelSketch::Core::Containers::Array<int> Values { 1, 2, 3 };
    VERIFY(Values.Size() == 3);
    VERIFY(Values[1] == 2);
    Values[1] = 4;
    VERIFY(Values[1] == 4);
    return true;
}

static bool Push()
{
    LevelSketch::Core::Containers::Array<int> Values;
    Values.Push(1);
    VERIFY(Values.Size() == 1);
    VERIFY(Values[0] == 1);
    return true;
}

static bool Pop()
{
    LevelSketch::Core::Containers::Array<int> Values { 1, 2, 3 };
    VERIFY(Values.Size() == 3);
    VERIFY(Values[0] == 1);
    VERIFY(Values[1] == 2);
    VERIFY(Values[2] == 3);
    Values.Pop();
    VERIFY(Values.Size() == 2);
    VERIFY(Values[0] == 1);
    VERIFY(Values[1] == 2);
    Values.Pop();
    VERIFY(Values.Size() == 1);
    VERIFY(Values[0] == 1);
    Values.Pop();
    VERIFY(Values.Size() == 0);
    Values.Pop();
    VERIFY(Values.Size() == 0);
    VERIFY(Values.Capacity() != 0);
    return true;
}

static bool Clear()
{
    LevelSketch::Core::Containers::Array<int> Values { 1, 2, 3 };
    VERIFY(Values.Size() == 3);
    VERIFY(Values[0] == 1);
    VERIFY(Values[1] == 2);
    VERIFY(Values[2] == 3);
    Values.Clear();
    VERIFY(Values.Capacity() == 0);
    VERIFY(Values.Size() == 0);
    VERIFY(Values.Data() == nullptr);
    return true;
}

static bool Objects()
{
    static i32 Constructed { 0 };

    class Object
    {
    public:
        Object() { Constructed++; }
        Object(const Object&) { Constructed++; }
        ~Object() { Constructed--; }
        Object& operator=(const Object&) { Constructed++; return *this; }
    };

    LevelSketch::Core::Containers::Array<Object> Objects;
    VERIFY(Constructed == 0);
    Objects.Push({});
    VERIFY(Constructed == 1);
    Objects.Remove(0);
    VERIFY(Constructed == 0);

    return true;
}

TestSuite* Array()
{
    return new TestSuite("Array", {
        TEST_CASE(Empty),
        TEST_CASE(Copy),
        TEST_CASE(Move),
        TEST_CASE(InitializerList),
        TEST_CASE(AssignCopy),
        TEST_CASE(AssignMove),
        TEST_CASE(Subscript),
        TEST_CASE(Push),
        TEST_CASE(Pop),
        TEST_CASE(Clear),
        TEST_CASE(Objects)
    });
}

}
}
}

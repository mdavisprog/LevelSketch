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
#include "../../Core/Containers/Array.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

class ArrayElement
{
public:
    ArrayElement(const ArrayElement& Other) = default;
    ArrayElement& operator=(const ArrayElement& Other) = default;

    ArrayElement()
    {
        m_Count++;
    }

    ArrayElement(ArrayElement&& Other)
        : m_Count(Other.m_Count)
    {
        Other.m_Count = 0;
    }

    u32 m_Count { 0 };
};

static bool Empty()
{
    Array<int> Values;
    VERIFY(Values.Capacity() == 0);
    VERIFY(Values.Size() == 0);
    VERIFY(Values.Data() == nullptr);
    return true;
}

static bool Copy()
{
    Array<int> Values { 1, 2, 3 };
    Array<int> ValuesCopy { Values };
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
    Array<int> Values { 1, 2, 3 };
    VERIFY(Values.Size() == 3);
    VERIFY(Values[0] == 1);
    VERIFY(Values[1] == 2);
    VERIFY(Values[2] == 3);
    Array<int> ValuesMove { std::move(Values) };
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
    Array<int> Values { 1, 2, 3 };
    VERIFY(Values.Size() == 3);
    VERIFY(Values[0] == 1);
    VERIFY(Values[1] == 2);
    VERIFY(Values[2] == 3);
    return true;
}

static bool AssignCopy()
{
    Array<int> Values { 1, 2, 3 };
    VERIFY(Values.Size() == 3);
    Array<int> ValuesCopy;
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
    Array<int> Values { 1, 2, 3 };
    VERIFY(Values.Size() == 3);
    Array<int> ValuesMove;
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
    Array<int> Values { 1, 2, 3 };
    VERIFY(Values.Size() == 3);
    VERIFY(Values[1] == 2);
    Values[1] = 4;
    VERIFY(Values[1] == 4);
    return true;
}

static bool Push()
{
    Array<int> Values;
    Values.Push(1);
    VERIFY(Values.Size() == 1);
    VERIFY(Values[0] == 1);
    return true;
}

static bool Pop()
{
    Array<int> Values { 1, 2, 3 };
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
    Array<int> Values { 1, 2, 3 };
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

    Array<Object> Objects;
    VERIFY(Constructed == 0);
    Objects.Push({});
    VERIFY(Constructed == 1);
    Objects.Remove(0);
    VERIFY(Constructed == 0);

    return true;
}

static bool MoveValue()
{
    Array<ArrayElement> Elements {};
    ArrayElement Element1;
    VERIFY(Element1.m_Count == 1);

    Elements.Push(Element1);
    VERIFY(Element1.m_Count == 1);
    VERIFY(Elements[0].m_Count == 1);

    ArrayElement Element2;
    VERIFY(Element2.m_Count == 1);

    Elements.Push(std::move(Element2));
    VERIFY(Elements[1].m_Count == 1);
    VERIFY(Element2.m_Count == 0);
    return true;
}

static bool Equality()
{
    Array<int> A { 1, 2, 3 };
    Array<int> B { 1, 2, 3 };
    VERIFY(A == B);
    B.Pop();
    VERIFY(A != B);
    return true;
}

static bool ElementDtor()
{
    static u32 s_Counter { 0 };
    static Array<u32> s_Deleted {};

    class Object
    {
    public:
        u32 m_Counter { 0 };

        Object()
            : m_Counter(++s_Counter)
        {
        }

        ~Object()
        {
            s_Deleted.Push(m_Counter);
        }
    };

    Array<Object> Objects { {}, {} };
    VERIFY(Objects[0].m_Counter == 1);
    VERIFY(Objects[1].m_Counter == 2);

    s_Deleted.Clear();
    Objects.Remove(1);
    VERIFY(s_Deleted[0] == 2);

    return true;
}

static bool RemoveItem()
{
    Array<int> Values { 100, 200, 300 };
    Values.Remove(200);
    VERIFY(Values.Size() == 2);
    VERIFY(Values[0] == 100);
    VERIFY(Values[1] == 300);
    return true;
}

static bool RemoveRange()
{
    Array<int> Values { 1, 2, 3, 4, 5, 6, 7, 8 };
    VERIFY(Values.Size() == 8);
    Values.RemoveRange(3, 3);
    VERIFY(Values.Size() == 5);
    VERIFY(Values[0] == 1);
    VERIFY(Values[1] == 2);
    VERIFY(Values[2] == 3);
    VERIFY(Values[3] == 7);
    VERIFY(Values[4] == 8);
    return true;
}

static bool Resize()
{
    Array<int> Values { 1, 2, 3, 4, 5 };
    VERIFY(Values.Size() == 5);
    Values.Resize(3);
    VERIFY(Values.Size() == 3);
    VERIFY(Values[0] == 1);
    VERIFY(Values[1] == 2);
    VERIFY(Values[2] == 3);
    Values.Resize(5);
    VERIFY(Values.Size() == 5);
    return true;
}

static bool ResizeUniquePtrs()
{
    Array<UniquePtr<i32>> Values;
    Values.Resize(1);
    VERIFY(Values.Size() == 1);
    VERIFY(Values[0] == nullptr);
    return true;
}

static bool ReserveUniquePtrs()
{
    Array<UniquePtr<i32>> Values;
    // Reserving does not in-place construct any elements. The
    // UniquePtr at index 0 has invalid data and will cause undefined
    // behavior if not properly created.
    Values.Reserve(1);
    VERIFY(Values.IsEmpty());
    Values.Push(nullptr);
    VERIFY(Values[0] == nullptr);
    Values.Clear();
    return true;
}

static bool Add()
{
    Array<i32> A { 1, 2, 3 };
    Array<i32> B { 4, 5, 6 };
    Array<i32> C { A + B };
    VERIFY(C.Size() == 6);
    VERIFY(C == Array<i32>({1, 2, 3, 4, 5, 6}));
    return true;
}

static bool Append()
{
    Array<i32> A { 1, 2, 3 };
    Array<i32> B { 4, 5, 6 };
    A += B;
    VERIFY(A.Size() == 6);
    VERIFY(A == Array<i32>({1, 2, 3, 4, 5, 6}));
    return true;
}

UniquePtr<TestSuite> ArrayTests()
{
    return TestSuite::New("Array", {
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
        TEST_CASE(Objects),
        TEST_CASE(MoveValue),
        TEST_CASE(Equality),
        TEST_CASE(ElementDtor),
        TEST_CASE(RemoveItem),
        TEST_CASE(RemoveRange),
        TEST_CASE(Resize),
        TEST_CASE(ResizeUniquePtrs),
        TEST_CASE(ReserveUniquePtrs),
        TEST_CASE(Add),
        TEST_CASE(Append)
    });
}

}
}
}

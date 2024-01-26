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

#include "../../Core/Containers/RedBlackTree.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"
#include "Core.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

template<typename K, typename V>
using RedBlackTree = LevelSketch::Core::Containers::RedBlackTree<K, V>;

static bool Empty()
{
    RedBlackTree<i32, i32> Instance;
    VERIFY(Instance.IsEmpty());
    return true;
}

static bool Insert()
{
    RedBlackTree<i32, i32> Instance;
    Instance.Insert(10, 10);
    VERIFY(Instance.Root() != nullptr);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Key() == 10);
    VERIFY(Instance.Root()->Value() == 10);

    Instance.Insert(20, 20);
    VERIFY(Instance.Root()->Key() == 20);
    VERIFY(Instance.Root()->Value() == 20);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left() != nullptr);
    VERIFY(Instance.Root()->Left()->Key() == 10);
    VERIFY(Instance.Root()->Left()->Value() == 10);
    VERIFY(Instance.Root()->Left()->IsRed());
    VERIFY(Instance.Root()->Right() == nullptr);

    Instance.Insert(30, 30);
    VERIFY(Instance.Root()->Key() == 20);
    VERIFY(Instance.Root()->Value() == 20);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left() != nullptr);
    VERIFY(Instance.Root()->Left()->Key() == 10);
    VERIFY(Instance.Root()->Left()->Value() == 10);
    VERIFY(Instance.Root()->Left()->IsBlack());
    VERIFY(Instance.Root()->Right() != nullptr);
    VERIFY(Instance.Root()->Right()->Key() == 30);
    VERIFY(Instance.Root()->Right()->Value() == 30);
    VERIFY(Instance.Root()->Right()->IsBlack());

    Instance.Insert(40, 40);
    VERIFY(Instance.Root()->Key() == 20);
    VERIFY(Instance.Root()->Value() == 20);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left() != nullptr);
    VERIFY(Instance.Root()->Left()->Key() == 10);
    VERIFY(Instance.Root()->Left()->Value() == 10);
    VERIFY(Instance.Root()->Left()->IsBlack());
    VERIFY(Instance.Root()->Right() != nullptr);
    VERIFY(Instance.Root()->Right()->Key() == 40);
    VERIFY(Instance.Root()->Right()->Value() == 40);
    VERIFY(Instance.Root()->Right()->IsBlack());
    VERIFY(Instance.Root()->Right()->Left() != nullptr);
    VERIFY(Instance.Root()->Right()->Left()->Key() == 30);
    VERIFY(Instance.Root()->Right()->Left()->Value() == 30);
    VERIFY(Instance.Root()->Right()->Left()->IsRed());
    VERIFY(Instance.Root()->Right()->Right() == nullptr);

    Instance.Insert(50, 50);
    VERIFY(Instance.Root()->Key() == 40);
    VERIFY(Instance.Root()->Value() == 40);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left()->Key() == 20);
    VERIFY(Instance.Root()->Left()->Value() == 20);
    VERIFY(Instance.Root()->Left()->IsRed());
    VERIFY(Instance.Root()->Right()->Key() == 50);
    VERIFY(Instance.Root()->Right()->Value() == 50);
    VERIFY(Instance.Root()->Right()->IsBlack());
    VERIFY(Instance.Root()->Right()->Left() == nullptr);
    VERIFY(Instance.Root()->Right()->Right() == nullptr);
    VERIFY(Instance.Root()->Left()->Left()->Key() == 10);
    VERIFY(Instance.Root()->Left()->Left()->Value() == 10);
    VERIFY(Instance.Root()->Left()->Left()->IsBlack());
    VERIFY(Instance.Root()->Left()->Left()->Left() == nullptr);
    VERIFY(Instance.Root()->Left()->Left()->Right() == nullptr);
    VERIFY(Instance.Root()->Left()->Right()->Key() == 30);
    VERIFY(Instance.Root()->Left()->Right()->Value() == 30);
    VERIFY(Instance.Root()->Left()->Right()->IsBlack());
    VERIFY(Instance.Root()->Left()->Right()->Left() == nullptr);
    VERIFY(Instance.Root()->Left()->Right()->Right() == nullptr);

    Instance.Insert(25, 25);
    VERIFY(Instance.Root()->Key() == 40);
    VERIFY(Instance.Root()->Value() == 40);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left()->Key() == 20);
    VERIFY(Instance.Root()->Left()->Value() == 20);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Right()->Key() == 50);
    VERIFY(Instance.Root()->Right()->Value() == 50);
    VERIFY(Instance.Root()->Right()->IsBlack());
    VERIFY(Instance.Root()->Right()->Left() == nullptr);
    VERIFY(Instance.Root()->Right()->Right() == nullptr);
    VERIFY(Instance.Root()->Left()->Left()->Key() == 10);
    VERIFY(Instance.Root()->Left()->Left()->Value() == 10);
    VERIFY(Instance.Root()->Left()->Left()->IsBlack());
    VERIFY(Instance.Root()->Left()->Left()->Left() == nullptr);
    VERIFY(Instance.Root()->Left()->Left()->Right() == nullptr);
    VERIFY(Instance.Root()->Left()->Right()->Key() == 30);
    VERIFY(Instance.Root()->Left()->Right()->Value() == 30);
    VERIFY(Instance.Root()->Left()->Right()->IsBlack());
    VERIFY(Instance.Root()->Left()->Right()->Left() != nullptr);
    VERIFY(Instance.Root()->Left()->Right()->Left()->Key() == 25);
    VERIFY(Instance.Root()->Left()->Right()->Left()->Value() == 25);
    VERIFY(Instance.Root()->Left()->Right()->Left()->IsRed());
    VERIFY(Instance.Root()->Left()->Right()->Right() == nullptr);
    return true;
}

static bool Delete()
{
    RedBlackTree<i32, i32> Instance;
    Instance.Insert(10, 10);
    Instance.Insert(20, 20);
    Instance.Insert(30, 30);
    Instance.Insert(40, 40);
    Instance.Insert(50, 50);
    Instance.Insert(25, 25);

    VERIFY(Instance.Root()->Key() == 40);
    VERIFY(Instance.Root()->Value() == 40);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left()->Key() == 20);
    VERIFY(Instance.Root()->Left()->Value() == 20);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Right()->Key() == 50);
    VERIFY(Instance.Root()->Right()->Value() == 50);
    VERIFY(Instance.Root()->Right()->IsBlack());
    VERIFY(Instance.Root()->Right()->Left() == nullptr);
    VERIFY(Instance.Root()->Right()->Right() == nullptr);
    VERIFY(Instance.Root()->Left()->Left()->Key() == 10);
    VERIFY(Instance.Root()->Left()->Left()->Value() == 10);
    VERIFY(Instance.Root()->Left()->Left()->IsBlack());
    VERIFY(Instance.Root()->Left()->Left()->Left() == nullptr);
    VERIFY(Instance.Root()->Left()->Left()->Right() == nullptr);
    VERIFY(Instance.Root()->Left()->Right()->Key() == 30);
    VERIFY(Instance.Root()->Left()->Right()->Value() == 30);
    VERIFY(Instance.Root()->Left()->Right()->IsBlack());
    VERIFY(Instance.Root()->Left()->Right()->Left() != nullptr);
    VERIFY(Instance.Root()->Left()->Right()->Left()->Key() == 25);
    VERIFY(Instance.Root()->Left()->Right()->Left()->Value() == 25);
    VERIFY(Instance.Root()->Left()->Right()->Left()->IsRed());
    VERIFY(Instance.Root()->Left()->Right()->Right() == nullptr);

    Instance.Delete(20);
    VERIFY(Instance.Root()->Key() == 40);
    VERIFY(Instance.Root()->Value() == 40);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left()->Key() == 25);
    VERIFY(Instance.Root()->Left()->Value() == 25);
    VERIFY(Instance.Root()->Left()->IsRed());
    VERIFY(Instance.Root()->Right()->Key() == 50);
    VERIFY(Instance.Root()->Right()->Value() == 50);
    VERIFY(Instance.Root()->Right()->IsBlack());
    VERIFY(Instance.Root()->Right()->Left() == nullptr);
    VERIFY(Instance.Root()->Right()->Right() == nullptr);
    VERIFY(Instance.Root()->Left()->Left()->Key() == 10);
    VERIFY(Instance.Root()->Left()->Left()->Value() == 10);
    VERIFY(Instance.Root()->Left()->Left()->IsBlack());
    VERIFY(Instance.Root()->Left()->Left()->Left() == nullptr);
    VERIFY(Instance.Root()->Left()->Left()->Right() == nullptr);
    VERIFY(Instance.Root()->Left()->Right()->Key() == 30);
    VERIFY(Instance.Root()->Left()->Right()->Value() == 30);
    VERIFY(Instance.Root()->Left()->Right()->IsBlack());
    VERIFY(Instance.Root()->Left()->Right()->Left() == nullptr);
    VERIFY(Instance.Root()->Left()->Right()->Right() == nullptr);

    Instance.Delete(50);
    VERIFY(Instance.Root()->Key() == 25);
    VERIFY(Instance.Root()->Value() == 25);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left()->Key() == 10);
    VERIFY(Instance.Root()->Left()->Value() == 10);
    VERIFY(Instance.Root()->Left()->IsBlack());
    VERIFY(Instance.Root()->Right()->Key() == 40);
    VERIFY(Instance.Root()->Right()->Value() == 40);
    VERIFY(Instance.Root()->Right()->IsBlack());
    VERIFY(Instance.Root()->Right()->Left() != nullptr);
    VERIFY(Instance.Root()->Right()->Right() == nullptr);
    VERIFY(Instance.Root()->Right()->Left()->Key() == 30);
    VERIFY(Instance.Root()->Right()->Left()->Value() == 30);
    VERIFY(Instance.Root()->Right()->Left()->IsRed());

    Instance.Delete(10);
    VERIFY(Instance.Root()->Key() == 30);
    VERIFY(Instance.Root()->Value() == 30);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left()->Key() == 25);
    VERIFY(Instance.Root()->Left()->Value() == 25);
    VERIFY(Instance.Root()->Left()->IsBlack());
    VERIFY(Instance.Root()->Right()->Key() == 40);
    VERIFY(Instance.Root()->Right()->Value() == 40);
    VERIFY(Instance.Root()->Right()->IsBlack());

    Instance.Delete(30);
    VERIFY(Instance.Root()->Key() == 40);
    VERIFY(Instance.Root()->Value() == 40);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left()->Key() == 25);
    VERIFY(Instance.Root()->Left()->Value() == 25);
    VERIFY(Instance.Root()->Left()->IsRed());
    VERIFY(Instance.Root()->Right() == nullptr);

    Instance.Delete(40);
    VERIFY(Instance.Root()->Key() == 25);
    VERIFY(Instance.Root()->Value() == 25);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left() == nullptr);
    VERIFY(Instance.Root()->Right() == nullptr);

    Instance.Delete(50);
    VERIFY(Instance.Root()->Key() == 25);
    VERIFY(Instance.Root()->Value() == 25);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left() == nullptr);
    VERIFY(Instance.Root()->Right() == nullptr);

    Instance.Delete(25);
    VERIFY(Instance.IsEmpty());
    return true;
}

static bool InsertDuplicate()
{
    RedBlackTree<i32, i32> Instance;
    Instance.Insert(10, 10);
    VERIFY(Instance.Root()->Key() == 10);
    VERIFY(Instance.Root()->Value() == 10);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left() == nullptr);
    VERIFY(Instance.Root()->Right() == nullptr);

    Instance.Insert(10, 10);
    VERIFY(Instance.Root()->Key() == 10);
    VERIFY(Instance.Root()->Value() == 10);
    VERIFY(Instance.Root()->IsBlack());
    VERIFY(Instance.Root()->Left() == nullptr);
    VERIFY(Instance.Root()->Right() == nullptr);
    return true;
}

static bool Find()
{
    RedBlackTree<i32, i32> Instance;
    Instance.Insert(10, 10);
    Instance.Insert(20, 20);
    Instance.Insert(30, 20);
    VERIFY(*Instance.Find(10) == 10);
    VERIFY(*Instance.Find(20) == 20);
    VERIFY(*Instance.Find(30) == 20);
    VERIFY(Instance.Find(15) == nullptr);
    return true;
}

static bool Size()
{
    RedBlackTree<i32, i32> Instance;
    VERIFY(Instance.Size() == 0);
    Instance.Insert(10, 10);
    VERIFY(Instance.Size() == 1);
    Instance.Insert(20, 20);
    Instance.Insert(30, 30);
    Instance.Insert(15, 15);
    Instance.Insert(40, 40);
    VERIFY(Instance.Size() == 5);
    Instance.Delete(20);
    VERIFY(Instance.Size() == 4);
    Instance.Delete(20);
    VERIFY(Instance.Size() == 4);
    Instance.Delete(30);
    VERIFY(Instance.Size() == 3);
    Instance.Delete(40);
    VERIFY(Instance.Size() == 2);
    Instance.Delete(10);
    VERIFY(Instance.Size() == 1);
    Instance.Delete(15);
    VERIFY(Instance.Size() == 0);
    return true;
}

static bool UniquePtrs()
{
    static i32 Counter { 0 };
    class Object
    {
    public:
        i32 m_Counter { ++Counter };
    };
    RedBlackTree<i32, UniquePtr<Object>> Instance;
    Instance.Insert(1, UniquePtr<Object>::New());
    VERIFY((*Instance.Find(1))->m_Counter == 1);
    return true;
}

static bool MoveCtor()
{
    RedBlackTree<i32, i32> Instance1;
    Instance1.Insert(10, 10).Insert(20, 20).Insert(30, 20);
    VERIFY(Instance1.Size() == 3);
    VERIFY(*Instance1.Find(10) == 10);
    VERIFY(*Instance1.Find(20) == 20);
    VERIFY(*Instance1.Find(30) == 20);
    RedBlackTree<i32, i32> Instance2 { std::move(Instance1) };
    VERIFY(Instance1.Size() == 0);
    VERIFY(Instance2.Size() == 3);
    VERIFY(*Instance2.Find(10) == 10);
    VERIFY(*Instance2.Find(20) == 20);
    VERIFY(*Instance2.Find(30) == 20);
    return true;
}

static bool MoveAssign()
{
    RedBlackTree<i32, i32> Instance1;
    Instance1.Insert(10, 10).Insert(20, 20).Insert(30, 20);
    VERIFY(Instance1.Size() == 3);
    VERIFY(*Instance1.Find(10) == 10);
    VERIFY(*Instance1.Find(20) == 20);
    VERIFY(*Instance1.Find(30) == 20);
    RedBlackTree<i32, i32> Instance2;
    Instance2.Insert(1, 1).Insert(2, 2).Insert(3, 2).Insert(4, 4);
    VERIFY(Instance2.Size() == 4);
    VERIFY(*Instance2.Find(1) == 1);
    VERIFY(*Instance2.Find(2) == 2);
    VERIFY(*Instance2.Find(3) == 2);
    VERIFY(*Instance2.Find(4) == 4);
    Instance2 = std::move(Instance1);
    VERIFY(Instance1.Size() == 0);
    VERIFY(Instance2.Size() == 3);
    VERIFY(*Instance2.Find(10) == 10);
    VERIFY(*Instance2.Find(20) == 20);
    VERIFY(*Instance2.Find(30) == 20);
    return true;
}

UniquePtr<TestSuite> RedBlackTreeTests()
{
    return TestSuite::New("RedBlackTree",
        { TEST_CASE(Empty),
            TEST_CASE(Insert),
            TEST_CASE(Delete),
            TEST_CASE(InsertDuplicate),
            TEST_CASE(Find),
            TEST_CASE(Size),
            TEST_CASE(UniquePtrs),
            TEST_CASE(MoveCtor),
            TEST_CASE(MoveAssign) });
}

}
}
}

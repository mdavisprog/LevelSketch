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
#include "../../Core/Containers/List.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

static bool Empty()
{
    List<i32> Instance;
    VERIFY(Instance.IsEmpty());
    return true;
}

static bool InsertEnd()
{
    List<i32> Instance;
    Instance.InsertEnd(5);
    VERIFY(Instance.Size() == 1);
    VERIFY(Instance.Head()->Data() == 5);
    Instance.InsertEnd(7);
    Instance.InsertEnd(11);
    VERIFY(Instance.Size() == 3);
    VERIFY(Instance.Head()->Data() == 5);
    VERIFY(Instance.Head()->Next()->Data() == 7);
    VERIFY(Instance.Head()->Next()->Next()->Data() == 11);
    return true;
}

static bool InsertBeginning()
{
    List<i32> Instance;
    Instance.InsertBeginning(5);
    VERIFY(Instance.Size() == 1);
    VERIFY(Instance.Head()->Data() == 5);
    Instance.InsertBeginning(7);
    Instance.InsertBeginning(11);
    VERIFY(Instance.Size() == 3);
    VERIFY(Instance.Head()->Data() == 11);
    VERIFY(Instance.Head()->Next()->Data() == 7);
    VERIFY(Instance.Head()->Next()->Next()->Data() == 5);
    return true;
}

static bool ReverseTraversal()
{
    List<i32> Instance;
    Instance
        .InsertEnd(1)
        .InsertEnd(3)
        .InsertEnd(5)
        .InsertEnd(7);
    VERIFY(Instance.Size() == 4);
    VERIFY(Instance.Tail()->Data() == 7);
    VERIFY(Instance.Tail()->Previous()->Data() == 5);
    VERIFY(Instance.Tail()->Previous()->Previous()->Data() == 3);
    VERIFY(Instance.Tail()->Previous()->Previous()->Previous()->Data() == 1);
    return true;
}

static bool Delete()
{
    List<i32> Instance;
    Instance
        .InsertEnd(1)
        .InsertEnd(3)
        .InsertEnd(5)
        .InsertEnd(7);
    VERIFY(Instance.Size() == 4);
    VERIFY(Instance.Head()->Data() == 1);
    VERIFY(Instance.Head()->Next()->Data() == 3);
    VERIFY(Instance.Head()->Next()->Next()->Data() == 5);
    VERIFY(Instance.Head()->Next()->Next()->Next()->Data() == 7);
    VERIFY(Instance.Delete(5));
    VERIFY(Instance.Size() == 3);
    VERIFY(Instance.Head()->Data() == 1);
    VERIFY(Instance.Head()->Next()->Data() == 3);
    VERIFY(Instance.Head()->Next()->Next()->Data() == 7);
    VERIFY(!Instance.Delete(5));
    VERIFY(Instance.Size() == 3);
    VERIFY(Instance.Delete(1));
    VERIFY(Instance.Size() == 2);
    VERIFY(Instance.Head()->Data() == 3);
    VERIFY(Instance.Head()->Next()->Data() == 7);
    VERIFY(Instance.Delete(7));
    VERIFY(Instance.Size() == 1);
    VERIFY(Instance.Head()->Data() == 3);
    VERIFY(Instance.Delete(3));
    VERIFY(Instance.IsEmpty());
    VERIFY(Instance.Head() == nullptr);
    return true;
}

UniquePtr<TestSuite> ListTests()
{
    return TestSuite::New("List", {
        TEST_CASE(Empty),
        TEST_CASE(InsertEnd),
        TEST_CASE(InsertBeginning),
        TEST_CASE(ReverseTraversal),
        TEST_CASE(Delete)
    });
}

}
}
}

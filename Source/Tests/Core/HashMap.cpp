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
#include "../../Core/Containers/HashMap.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

using HashMapConstants = LevelSketch::Core::Containers::HashMapConstants;

template<typename T>
struct SimpleHash : LevelSketch::Core::Traits::Base<T>
{
public:
    static u64 Hash(const T& Value)
    {
        return static_cast<u64>(Value);
    }
};

static bool Empty()
{
    HashMap<i32, i32> Instance;
    VERIFY(Instance.IsEmpty());
    return true;
}

static bool Index()
{
    HashMap<i32, i32> Instance;
    VERIFY(Instance.IsEmpty());
    Instance[10] = 20;
    VERIFY(Instance[10] == 20);
    VERIFY(Instance.Size() == 1);
    return true;
}

static bool BucketGrow()
{
    class Constants : public HashMapConstants
    {
    public:
        static constexpr u64 BucketSize() { return 2; }
        static constexpr f64 GrowPercent() { return 1.0; }
        static constexpr f64 GrowFactor() { return 1.0; }
    };

    HashMap<i32, i32, SimpleHash<i32>, Constants> Instance;
    VERIFY(Instance.Capacity() == 2);
    Instance[1] = 1;
    VERIFY(Instance.Size() == 1);
    VERIFY(Instance.Capacity() == 2);
    VERIFY(Instance[1] == 1);
    Instance[2] = 2;
    VERIFY(Instance.Size() == 2);
    VERIFY(Instance.Capacity() == 4);
    VERIFY(Instance[1] == 1);
    VERIFY(Instance[2] == 2);
    return true;
}

static bool Remove()
{
    HashMap<i32, i32> Instance;
    Instance[20] = 20;
    VERIFY(Instance.Size() == 1);
    VERIFY(Instance[20] == 20);
    Instance[15] = 15;
    VERIFY(Instance.Size() == 2);
    VERIFY(Instance[20] == 20);
    VERIFY(Instance[15] == 15);
    VERIFY(Instance.Remove(20));
    VERIFY(Instance.Size() == 1);
    VERIFY(Instance[15] == 15);
    VERIFY(!Instance.Remove(20));
    VERIFY(Instance.Size() == 1);
    VERIFY(Instance[15] == 15);
    VERIFY(Instance.Remove(15));
    VERIFY(Instance.Size() == 0);
    VERIFY(!Instance.Remove(15));
    return true;
}

UniquePtr<TestSuite> HashMapTests()
{
    return TestSuite::New("HashMap", {
        TEST_CASE(Empty),
        TEST_CASE(Index),
        TEST_CASE(BucketGrow),
        TEST_CASE(Remove)
    });
}

}
}
}

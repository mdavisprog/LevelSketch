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
#include "../../Core/Optional.hpp"
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
    Optional<u32> Instance;
    VERIFY(!Instance.HasValue());
    return true;
}

static bool Construct()
{
    Optional<u32> Instance { 5 };
    VERIFY(Instance.HasValue());
    VERIFY(Instance.Value() == 5);
    return true;
}

static bool Clear()
{
    Optional<u32> Instance { 5 };
    VERIFY(Instance.HasValue());
    VERIFY(Instance.Value() == 5);
    Instance.Clear();
    VERIFY(!Instance.HasValue());
    return true;
}

static bool Assign()
{
    Optional<u32> Instance;
    VERIFY(!Instance.HasValue());
    Instance = 5;
    VERIFY(Instance.HasValue());
    VERIFY(Instance.Value() == 5);
    return true;
}

static bool CopyConstructor()
{
    Optional<u32> InstanceA { 5 };
    VERIFY(InstanceA.HasValue());
    VERIFY(InstanceA.Value() == 5);
    Optional<u32> InstanceB { InstanceA };
    VERIFY(InstanceA.HasValue());
    VERIFY(InstanceA.Value() == 5);
    VERIFY(InstanceB.HasValue());
    VERIFY(InstanceB.Value() == 5);
    return true;
}

static bool CopyAssign()
{
    Optional<u32> InstanceA { 5 };
    Optional<u32> InstanceB { 7 };
    VERIFY(InstanceA.HasValue());
    VERIFY(InstanceA.Value() == 5);
    VERIFY(InstanceB.HasValue());
    VERIFY(InstanceB.Value() == 7);
    InstanceB = InstanceA;
    VERIFY(InstanceA.HasValue());
    VERIFY(InstanceA.Value() == 5);
    VERIFY(InstanceB.HasValue());
    VERIFY(InstanceB.Value() == 5);
    return true;
}

UniquePtr<TestSuite> OptionalTests()
{
    return TestSuite::New("Optional", {
        TEST_CASE(Empty),
        TEST_CASE(Construct),
        TEST_CASE(Clear),
        TEST_CASE(Assign),
        TEST_CASE(CopyConstructor),
        TEST_CASE(CopyAssign)
    });
}

}
}
}

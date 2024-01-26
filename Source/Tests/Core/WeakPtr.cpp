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

#include "../../Core/Memory/WeakPtr.hpp"
#include "../../Core/Compiler.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"
#include "Core.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

class Object
{
public:
    Object()
    {
    }
};

static bool Empty()
{
    WeakPtr<Object> WeakPtr;
    VERIFY(!WeakPtr.IsValid());
    VERIFY(WeakPtr.GetReferenceCount() == 0);
    VERIFY(WeakPtr.Lock() == nullptr);
    return true;
}

static bool WeakLock()
{
    WeakPtr<Object> Weak;
    VERIFY(Weak.Lock() == nullptr);
    SharedPtr<Object> Shared { SharedPtr<Object>::New() };
    Weak = Shared;
    VERIFY(Weak.IsValid());
    VERIFY(Weak.GetReferenceCount() == 1);
    VERIFY(Weak.Lock() != nullptr);
    return true;
}

static bool WeakRefDestroyed()
{
    WeakPtr<Object> Weak;
    VERIFY(!Weak.IsValid());
    {
        SharedPtr<Object> Shared { SharedPtr<Object>::New() };
        Weak = Shared;
        VERIFY(Weak.IsValid());
    }
    VERIFY(!Weak.IsValid());
    return true;
}

static bool WeakCopy()
{
    WeakPtr<Object> WeakPtr1;
    VERIFY(!WeakPtr1.IsValid());
    {
        SharedPtr<Object> Shared { SharedPtr<Object>::New() };
        VERIFY(!WeakPtr1.IsValid());
        {
            WeakPtr<Object> WeakPtr2 { Shared };
            VERIFY(WeakPtr2.IsValid());
            WeakPtr1 = WeakPtr2;
            VERIFY(WeakPtr1.IsValid());
        }
        VERIFY(WeakPtr1.IsValid());
    }
    VERIFY(!WeakPtr1.IsValid());
    return true;
}

// clang-format off
#if defined(CLANG)
PUSH_DISABLE_WARNING(-Wself-assign-overloaded)
#endif
// clang-format on

static bool SelfCopy()
{
    SharedPtr<Object> Shared { SharedPtr<Object>::New() };
    WeakPtr<Object> Weak { Shared };
    VERIFY(Weak.IsValid());
    Weak = Weak;
    VERIFY(Weak.IsValid());
    VERIFY(Weak.GetReferenceCount() == 1);
    return true;
}

#if defined(CLANG)
POP_DISABLE_WARNING
#endif

UniquePtr<TestSuite> WeakPtrTests()
{
    return TestSuite::New("WeakPtr",
        { TEST_CASE(Empty),
            TEST_CASE(WeakLock),
            TEST_CASE(WeakRefDestroyed),
            TEST_CASE(WeakCopy),
            TEST_CASE(SelfCopy) });
}

}
}
}

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
#include "../../Core/Memory/WeakPtr.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

namespace Memory
{
    template<typename T>
    using SharedPtr = LevelSketch::Core::Memory::SharedPtr<T>;

    template<typename T>
    using WeakPtr = LevelSketch::Core::Memory::WeakPtr<T>;
}

class Object
{
public:
    Object() {}
};

static bool Empty()
{
    Memory::WeakPtr<Object> WeakPtr;
    VERIFY(!WeakPtr.IsValid());
    VERIFY(WeakPtr.GetReferenceCount() == 0);
    VERIFY(WeakPtr.Lock() == nullptr);
    return true;
}

static bool WeakLock()
{
    Memory::WeakPtr<Object> WeakPtr;
    VERIFY(WeakPtr.Lock() == nullptr);
    Memory::SharedPtr<Object> SharedPtr { Memory::SharedPtr<Object>::New() };
    WeakPtr = SharedPtr;
    VERIFY(WeakPtr.IsValid());
    VERIFY(WeakPtr.GetReferenceCount() == 1);
    VERIFY(WeakPtr.Lock() != nullptr);
    return true;
}

static bool WeakRefDestroyed()
{
    Memory::WeakPtr<Object> WeakPtr;
    VERIFY(!WeakPtr.IsValid());
    {
        Memory::SharedPtr<Object> SharedPtr { Memory::SharedPtr<Object>::New() };
        WeakPtr = SharedPtr;
        VERIFY(WeakPtr.IsValid());
    }
    VERIFY(!WeakPtr.IsValid());
    return true;
}

static bool WeakCopy()
{
    Memory::WeakPtr<Object> WeakPtr1;
    VERIFY(!WeakPtr1.IsValid());
    {
        Memory::SharedPtr<Object> SharedPtr { Memory::SharedPtr<Object>::New() };
        VERIFY(!WeakPtr1.IsValid());
        {
            Memory::WeakPtr<Object> WeakPtr2 { SharedPtr };
            VERIFY(WeakPtr2.IsValid());
            WeakPtr1 = WeakPtr2;
            VERIFY(WeakPtr1.IsValid());
        }
        VERIFY(WeakPtr1.IsValid());
    }
    VERIFY(!WeakPtr1.IsValid());
    return true;
}

static bool SelfCopy()
{
    Memory::SharedPtr<Object> SharedPtr { Memory::SharedPtr<Object>::New() };
    Memory::WeakPtr<Object> WeakPtr { SharedPtr };
    VERIFY(WeakPtr.IsValid());
    WeakPtr = WeakPtr;
    VERIFY(WeakPtr.IsValid());
    VERIFY(WeakPtr.GetReferenceCount() == 1);
    return true;
}

TestSuite* WeakPtr()
{
    return new TestSuite("WeakPtr", {
        TEST_CASE(Empty),
        TEST_CASE(WeakLock),
        TEST_CASE(WeakRefDestroyed),
        TEST_CASE(WeakCopy),
        TEST_CASE(SelfCopy)
    });
}

}
}
}

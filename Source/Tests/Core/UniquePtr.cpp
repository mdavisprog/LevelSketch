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
#include "../../Core/Memory/UniquePtr.hpp"
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
    using UniquePtr = LevelSketch::Core::Memory::UniquePtr<T>;
}

static bool CreateNull()
{
    Memory::UniquePtr<int> Instance;
    VERIFY(Instance.Get() == nullptr);
    return true;
}

static bool CreateInstance()
{
    Memory::UniquePtr<int> Instance;
    VERIFY(!Instance.IsValid());
    Instance = Memory::UniquePtr<int>::New();
    VERIFY(Instance.IsValid());
    return true;
}

static bool Move()
{
    Memory::UniquePtr<int> Instance1 { Memory::UniquePtr<int>::New() };
    VERIFY(Instance1.IsValid());
    Memory::UniquePtr<int> Instance2;
    VERIFY(!Instance2.IsValid());
    Instance2 = std::move(Instance1);
    VERIFY(!Instance1.IsValid());
    VERIFY(Instance2.IsValid());
    return true;
}

static bool Polymorphism()
{
    struct Base {};
    struct Derived : public Base {};
    Core::Memory::UniquePtr<Base> Object = Core::Memory::UniquePtr<Derived>::New();
    // No need for any verifies. This test is meant to verify that UniquePtr is set up
    // to allow for base types to store pointers to derived types.
    return true;
}

Memory::UniquePtr<TestSuite> UniquePtr()
{
    return TestSuite::New("UniquePtr", {
        TEST_CASE(CreateNull),
        TEST_CASE(CreateInstance),
        TEST_CASE(Move),
        TEST_CASE(Polymorphism)
    });
}

}
}
}

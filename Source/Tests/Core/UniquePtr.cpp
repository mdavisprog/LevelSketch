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

static bool CreateNull()
{
    UniquePtr<int> Instance;
    VERIFY(Instance == nullptr);
    UniquePtr<int> Instance2 { nullptr };
    VERIFY(Instance2 == nullptr);
    return true;
}

static bool CreateInstance()
{
    UniquePtr<int> Instance;
    VERIFY(!Instance.IsValid());
    Instance = UniquePtr<int>::New();
    VERIFY(Instance.IsValid());
    return true;
}

static bool Adopt()
{
    int* Value = new int(5);
    VERIFY(*Value == 5);
    UniquePtr<int> Adopted { UniquePtr<int>::Adopt(Value) };
    VERIFY(*Adopted == 5);
    return true;
}

static bool NullAssignment()
{
    UniquePtr<int> Instance { UniquePtr<int>::New(5) };
    VERIFY(*Instance == 5);
    Instance = nullptr;
    VERIFY(!Instance.IsValid());
    return true;
}

static bool Move()
{
    UniquePtr<int> Instance1 { UniquePtr<int>::New() };
    VERIFY(Instance1.IsValid());
    UniquePtr<int> Instance2;
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
    UniquePtr<Base> Object = UniquePtr<Derived>::New();
    // No need for any verifies. This test is meant to verify that UniquePtr is set up
    // to allow for base types to store pointers to derived types.
    return true;
}

static bool Equality()
{
    UniquePtr<int> A { UniquePtr<int>::New(5) };
    UniquePtr<int> B { UniquePtr<int>::New(6) };
    VERIFY(A.IsValid());
    VERIFY(B.IsValid());
    VERIFY(A != B);
    VERIFY(*A != *B);
    UniquePtr<int>& C { A };
    int* D { B.Get() };
    VERIFY(A == C);
    VERIFY(*A == *C);
    VERIFY(A != D);
    VERIFY(B == D);
    VERIFY(C != B);
    return true;
}

UniquePtr<TestSuite> UniquePtrTests()
{
    return TestSuite::New("UniquePtr", {
        TEST_CASE(CreateNull),
        TEST_CASE(CreateInstance),
        TEST_CASE(Adopt),
        TEST_CASE(NullAssignment),
        TEST_CASE(Move),
        TEST_CASE(Polymorphism),
        TEST_CASE(Equality)
    });
}

}
}
}

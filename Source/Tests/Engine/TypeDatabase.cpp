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

#include "Engine.hpp"
#include "../../Engine/TypeDatabase.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Engine
{

using Type = LevelSketch::Engine::Type;
using TypeDatabase = LevelSketch::Engine::TypeDatabase;

class TypeDatabaseScope
{
public:
    ~TypeDatabaseScope()
    {
        TypeDatabase::Instance().Clear();
    }
};

static bool Root()
{
    TypeDatabaseScope Scope;
    VERIFY(TypeDatabase::Instance().HasType("Type"));
    return true;
}

static bool Base()
{
    TypeDatabaseScope Scope;

    class Base
    {
    public:
        Base() = default;
    };

    VERIFY(!TypeDatabase::Instance().HasType("Base"));
    DECLARE_BASE_TYPE(Base);
    VERIFY(TypeDatabase::Instance().HasType("Base"));
    return true;
}

static bool Instance()
{
    TypeDatabaseScope Scope;

    class Base
    {
    public:
        Base() = default;
    };

    Type const* BaseTypePtr { DECLARE_BASE_TYPE(Base) };
    VERIFY(BaseTypePtr != nullptr);
    VERIFY(TypeDatabase::Instance().HasType("Base"));
    UniquePtr<Base> Instance { BaseTypePtr->NewUnique<Base>() };
    VERIFY(Instance != nullptr);
    return true;
}

static bool Child()
{
    TypeDatabaseScope Scope;

    class Parent {};
    class Child : public Parent {};
    DECLARE_BASE_TYPE(Parent);
    VERIFY(TypeDatabase::Instance().HasType("Parent"));
    VERIFY(!TypeDatabase::Instance().HasType("Child"));
    DECLARE_TYPE(Child, Parent);
    VERIFY(TypeDatabase::Instance().HasType("Child"));
    return true;
}

static bool Inherits()
{
    TypeDatabaseScope Scope;

    class A {};
    class B : public A {};
    class C : public A {};

    DECLARE_BASE_TYPE(A);
    DECLARE_TYPE(B, A);
    DECLARE_TYPE(C, A);

    VERIFY(TypeDatabase::Instance().HasType("A"));
    VERIFY(TypeDatabase::Instance().HasType("B"));
    VERIFY(TypeDatabase::Instance().HasType("C"));

    VERIFY(TYPE_INHERITS(B, A));
    VERIFY(!TYPE_INHERITS(C, B));

    return true;
}

UniquePtr<TestSuite> TypeDatabaseTests()
{
    return TestSuite::New("TypeDatabase", {
        TEST_CASE(Root),
        TEST_CASE(Base),
        TEST_CASE(Instance),
        TEST_CASE(Child),
        TEST_CASE(Inherits)
    });
}

}
}
}

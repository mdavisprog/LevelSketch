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
#include "../../Engine/Class.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Engine
{

using Class = LevelSketch::Engine::Class;
using TypeDatabase = LevelSketch::Engine::TypeDatabase;

static bool BaseClass()
{
    Class::StaticInitialize(true);

    class Base : public Class
    {
        DECLARE_CLASS(Base, Class)
    };

    VERIFY(!TYPE_EXISTS(Base));
    REGISTER_CLASS(Base);
    VERIFY(TYPE_EXISTS(Base));
    TypeDatabase::Instance().Clear();
    return true;
}

static bool DerivedClass()
{
    Class::StaticInitialize(true);

    class Base : public Class { DECLARE_CLASS(Base, Class) };
    class Derived : public Base { DECLARE_CLASS(Derived, Base) };

    VERIFY(!TYPE_EXISTS(Base));
    VERIFY(!TYPE_EXISTS(Derived));

    REGISTER_CLASS(Base);
    VERIFY(TYPE_EXISTS(Base));
    VERIFY(!TYPE_EXISTS(Derived));

    REGISTER_CLASS(Derived);
    VERIFY(TYPE_EXISTS(Derived));

    TypeDatabase::Instance().Clear();
    return true;
}

static bool CastClass()
{
    Class::StaticInitialize(true);

    class A : public Class { DECLARE_CLASS(A, Class) };
    class B : public A { DECLARE_CLASS(B, A) };
    class C : public A { DECLARE_CLASS(C, A) };
    class D : public B { DECLARE_CLASS(D, B) };

    VERIFY(!TYPE_EXISTS(A));
    VERIFY(!TYPE_EXISTS(B));
    VERIFY(!TYPE_EXISTS(C));
    VERIFY(!TYPE_EXISTS(D));

    REGISTER_CLASS(A);
    REGISTER_CLASS(B);
    REGISTER_CLASS(C);
    REGISTER_CLASS(D);

    VERIFY(TYPE_EXISTS(A));
    VERIFY(TYPE_EXISTS(B));
    VERIFY(TYPE_EXISTS(C));
    VERIFY(TYPE_EXISTS(D));

    UniquePtr<A> Instance { B::Instance() };

    VERIFY(Instance->GetType() == B::ClassType());
    VERIFY(Instance->GetType() != C::ClassType());

    A* Casted = Cast<B>(Instance.Get());
    VERIFY(Casted != nullptr);

    Casted = Cast<C>(Instance.Get());
    VERIFY(Casted == nullptr);

    Casted = Cast<D>(Instance.Get());
    VERIFY(Casted == nullptr);

    TypeDatabase::Instance().Clear();
    return true;
}

UniquePtr<TestSuite> ClassTests()
{
    return TestSuite::New("Class", {
        TEST_CASE(BaseClass),
        TEST_CASE(DerivedClass),
        TEST_CASE(CastClass)
    });
}

}
}
}

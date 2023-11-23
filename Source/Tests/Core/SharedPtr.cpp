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
#include "../../Core/Containers/Array.hpp"
#include "../../Core/Memory/SharedPtr.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Core
{

class Object
{
public:
    static u32 s_Counter;

    Object()
    {
        s_Counter++;
    }

    ~Object()
    {
        s_Counter--;
    }
};

u32 Object::s_Counter { 0 };

static bool CreateDestroy()
{
    Object::s_Counter = 0;
    VERIFY(Object::s_Counter == 0);
    {
        LevelSketch::Core::Memory::SharedPtr<Object> Object_ { LevelSketch::Core::Memory::SharedPtr<Object>::New() };
        VERIFY(Object::s_Counter == 1 && Object_.GetReferenceCount() == 1);
    }
    VERIFY(Object::s_Counter == 0);
    return true;
}

static bool CreateCopy()
{
    Object::s_Counter = 0;
    VERIFY(Object::s_Counter == 0);
    {
        LevelSketch::Core::Memory::SharedPtr<Object> Object_1 { LevelSketch::Core::Memory::SharedPtr<Object>::New() };
        VERIFY(Object::s_Counter == 1);
        VERIFY(Object_1.GetReferenceCount() == 1);
        {
            LevelSketch::Core::Memory::SharedPtr<Object> Object_2 { Object_1 };
            VERIFY(Object::s_Counter == 1);
            VERIFY(Object_1.GetReferenceCount() == 2);
            VERIFY(Object_2.GetReferenceCount() == 2);
        }
        VERIFY(Object::s_Counter == 1);
        VERIFY(Object_1.GetReferenceCount() == 1);
    }
    VERIFY(Object::s_Counter == 0);
    return true;
}

static bool ArrayPtrs()
{
    LevelSketch::Core::Memory::SharedPtr<int> Value { LevelSketch::Core::Memory::SharedPtr<int>::New(1) };
    VERIFY(*Value == 1);
    LevelSketch::Core::Containers::Array<LevelSketch::Core::Memory::SharedPtr<int>> Values { Value, Value, LevelSketch::Core::Memory::SharedPtr<int>::New(5) };
    VERIFY(Value.GetReferenceCount() == 3);
    VERIFY(*Values[0] == 1 && Values[0].GetReferenceCount() == 3);
    VERIFY(*Values[1] == 1 && Values[1].GetReferenceCount() == 3);
    VERIFY(*Values[2] == 5 && Values[2].GetReferenceCount() == 1);
    Values.Clear();
    VERIFY(Value.GetReferenceCount() == 1);
    return true;
}

static bool Move()
{
    LevelSketch::Core::Memory::SharedPtr<int> Value { LevelSketch::Core::Memory::SharedPtr<int>::New(1) };
    VERIFY(*Value == 1 && Value.GetReferenceCount() == 1);
    LevelSketch::Core::Memory::SharedPtr<int> Moved { std::move(Value) };
    VERIFY(*Moved == 1 && Moved.GetReferenceCount() == 1);
    VERIFY(Value.IsNull());
    VERIFY(Value.GetReferenceCount() == 0);
    return true;
}

TestSuite* SharedPtr()
{
    #define TEST_CASE(Fn) {#Fn, Fn}

    return new TestSuite("SharedPtr", {
        TEST_CASE(CreateDestroy),
        TEST_CASE(CreateCopy),
        TEST_CASE(ArrayPtrs),
        TEST_CASE(Move)
    });

    #undef TEST_CASE
}

}
}
}

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

namespace Containers
{
    template<typename T>
    using Array = LevelSketch::Core::Containers::Array<T>;
}

namespace Memory
{
    template<typename T>
    using SharedPtr = LevelSketch::Core::Memory::SharedPtr<T>;
}

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

static bool CreateNull()
{
    Memory::SharedPtr<Object> Instance1 {};
    VERIFY(Instance1.IsNull());
    VERIFY(Instance1.GetReferenceCount() == 0);

    Memory::SharedPtr<Object> Instance2 { nullptr };
    VERIFY(Instance2.IsNull());
    VERIFY(Instance2.GetReferenceCount() == 0);
    return true;
}

static bool CreateDestroy()
{
    Object::s_Counter = 0;
    VERIFY(Object::s_Counter == 0);
    {
        Memory::SharedPtr<Object> Object_ { Memory::SharedPtr<Object>::New() };
        VERIFY(Object::s_Counter == 1);
        VERIFY(Object_.GetReferenceCount() == 1);
    }
    VERIFY(Object::s_Counter == 0);
    return true;
}

static bool CreateCopy()
{
    Object::s_Counter = 0;
    VERIFY(Object::s_Counter == 0);
    {
        Memory::SharedPtr<Object> Object_1 { Memory::SharedPtr<Object>::New() };
        VERIFY(Object::s_Counter == 1);
        VERIFY(Object_1.GetReferenceCount() == 1);
        {
            Memory::SharedPtr<Object> Object_2 { Object_1 };
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
    Memory::SharedPtr<int> Value { Memory::SharedPtr<int>::New(1) };
    VERIFY(*Value == 1);
    Containers::Array<Memory::SharedPtr<int>> Values { Value, Value, Memory::SharedPtr<int>::New(5) };
    VERIFY(Value.GetReferenceCount() == 3);
    VERIFY(*Values[0] == 1);
    VERIFY(*Values[1] == 1);
    VERIFY(*Values[2] == 5);
    VERIFY(Values[0].GetReferenceCount() == 3);
    VERIFY(Values[1].GetReferenceCount() == 3);
    VERIFY(Values[2].GetReferenceCount() == 1);
    Values.Clear();
    VERIFY(Value.GetReferenceCount() == 1);
    return true;
}

static bool Move()
{
    Memory::SharedPtr<int> Value { Memory::SharedPtr<int>::New(1) };
    VERIFY(*Value == 1);
    VERIFY(Value.GetReferenceCount() == 1);
    Memory::SharedPtr<int> Moved { std::move(Value) };
    VERIFY(*Moved == 1);
    VERIFY(Moved.GetReferenceCount() == 1);
    VERIFY(Value.IsNull());
    VERIFY(Value.GetReferenceCount() == 0);
    return true;
}

static bool Equality()
{
    Memory::SharedPtr<Object> Value1 { Memory::SharedPtr<Object>::New() };
    Memory::SharedPtr<Object> Value2 { Value1 };
    VERIFY(Value1 == Value2);
    VERIFY(Value1 != nullptr);
    Value1 = nullptr;
    VERIFY(Value1 != Value2);
    VERIFY(Value1 == nullptr);
    return true;
}

static bool SelfCopy()
{
    Memory::SharedPtr<Object> Value1 { Memory::SharedPtr<Object>::New() };
    VERIFY(Value1.GetReferenceCount() == 1);
    Value1 = Value1;
    VERIFY(Value1.GetReferenceCount() == 1);
    return true;
}

static bool SetNull()
{
    Memory::SharedPtr<Object> Value1;
    VERIFY(Value1.GetReferenceCount() == 0);
    Memory::SharedPtr<Object> Value2 { Memory::SharedPtr<Object>::New() };
    VERIFY(Value2.GetReferenceCount() == 1);
    Value1 = Value2;
    VERIFY(Value1.GetReferenceCount() == 2);
    Value1 = nullptr;
    VERIFY(Value2.GetReferenceCount() == 1);
    VERIFY(Value1.GetReferenceCount() == 0);
    return true;
}

TestSuite* SharedPtr()
{
    return new TestSuite("SharedPtr", {
        TEST_CASE(CreateNull),
        TEST_CASE(CreateDestroy),
        TEST_CASE(CreateCopy),
        TEST_CASE(ArrayPtrs),
        TEST_CASE(Move),
        TEST_CASE(Equality),
        TEST_CASE(SelfCopy),
        TEST_CASE(SetNull)
    });
}

}
}
}

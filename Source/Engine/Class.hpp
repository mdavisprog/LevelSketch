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

#pragma once

#include "TypeDatabase.hpp"

#include <type_traits>

namespace LevelSketch
{
namespace Engine
{

class Class
{
public:
    //
    // GetType and StaticInitialize offer an optional 'Refresh' argument.
    // This is used in the testing framework to allow for re-registering
    // this class to ensure a clean TypeDatabase for each test case.
    //

    static Type* ClassType(bool Refresh = false)
    {
        static Type* s_Type { nullptr };
        if (s_Type == nullptr || Refresh)
        {
            s_Type = TypeDatabase::Instance().GetType("Class");
            LS_ASSERT(s_Type != nullptr);
        }
        return s_Type;
    }

    static void StaticInitialize(bool Refresh = false)
    {
        static bool Initialized { false };
        if (Initialized && !Refresh)
        {
            return;
        }
        Initialized = true;
        DECLARE_BASE_TYPE(Class);
        ClassType(Refresh); // Cache the pointer to the Type.
    }

    template<typename T>
    static T* Cast(Class* Object)
    {
        if (Object == nullptr)
        {
            return nullptr;
        }

        Type* ObjectType { Object->GetType() };

        if (ObjectType != T::ClassType())
        {
            Type* Parent = Object->GetParentType();

            while (Parent != nullptr && Parent != ObjectType)
            {
                Parent = Parent->Parent();
            }

            if (Parent == nullptr)
            {
                return nullptr;
            }
        }

        return static_cast<T*>(Object);
    }

    template<typename T>
    static T* Cast(UniquePtr<T>& Object)
    {
        return Cast<T>(Object.Get());
    }

    virtual Type* GetType() const
    {
        return ClassType();
    }

    virtual Type* GetParentType() const
    {
        return nullptr;
    }

    Class()
    {
    }

    virtual ~Class() = default;
};

}

template<typename T>
static T* Cast(Engine::Class* Object)
{
    return Engine::Class::Cast<T>(Object);
}

}

#define DECLARE_CLASS(CLASS, PARENT)                                                                                   \
public:                                                                                                                \
    static void StaticInitialize()                                                                                     \
    {                                                                                                                  \
        static_assert(std::is_base_of_v<LevelSketch::Engine::Class, PARENT>,                                           \
            "Parent class (" #PARENT ") is not a child of LevelSketch::Engine::Class!");                               \
        static bool Initialized { false };                                                                             \
        if (Initialized)                                                                                               \
        {                                                                                                              \
            return;                                                                                                    \
        }                                                                                                              \
        Initialized = true;                                                                                            \
        DECLARE_TYPE(CLASS, PARENT);                                                                                   \
        ClassType();                                                                                                   \
    }                                                                                                                  \
    static LevelSketch::Engine::Type* ClassType()                                                                      \
    {                                                                                                                  \
        static LevelSketch::Engine::Type* s_Type { nullptr };                                                          \
        if (s_Type == nullptr)                                                                                         \
        {                                                                                                              \
            s_Type = LevelSketch::Engine::TypeDatabase::Instance().GetType(#CLASS);                                    \
            LS_ASSERTF(s_Type != nullptr, "Class (%s) is not registerd. Make sure REGISTER_CLASS is called.", #CLASS); \
        }                                                                                                              \
        return s_Type;                                                                                                 \
    }                                                                                                                  \
    static LevelSketch::Core::Memory::UniquePtr<CLASS> Instance()                                                      \
    {                                                                                                                  \
        return ClassType()->NewUnique<CLASS>();                                                                        \
    }                                                                                                                  \
    virtual LevelSketch::Engine::Type* GetType() const override                                                        \
    {                                                                                                                  \
        return ClassType();                                                                                            \
    }                                                                                                                  \
    virtual LevelSketch::Engine::Type* GetParentType() const override                                                  \
    {                                                                                                                  \
        return PARENT::GetType();                                                                                      \
    }                                                                                                                  \
                                                                                                                       \
private:

#define REGISTER_CLASS(Class) Class::StaticInitialize()

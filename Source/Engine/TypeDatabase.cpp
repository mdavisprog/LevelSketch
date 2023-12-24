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

#include "TypeDatabase.hpp"
#include "../Core/Assert.hpp"

namespace LevelSketch
{
namespace Engine
{

TypeDatabase& TypeDatabase::Instance()
{
    static TypeDatabase Instance;
    return Instance;
}

bool TypeDatabase::HasType(const char* Name) const
{
    return HasType(m_Root.Get(), Name);
}

bool TypeDatabase::Inherits(const char* Name, const char* Parent) const
{
    Type* Type_ { GetType(m_Root.Get(), Name) };

    if (Type_ == nullptr || Type_->m_Parent == nullptr)
    {
        return false;
    }

    if (Type_->m_Parent->m_Name != Parent)
    {
        return false;
    }

    return true;
}

TypeDatabase::TypeDatabase()
{
    m_Root = UniquePtr<Type>::New("Type");
}

Type* TypeDatabase::InternalRegister(const char* Name, Type::OnNewSignature&& Fn)
{
    Type* Registered { GetType(m_Root.Get(), Name) };

    if (Registered != nullptr)
    {
        return Registered;
    }

    UniquePtr<Type> Child { UniquePtr<Type>::New(Name) };
    Child->m_Parent = m_Root.Get();
    Child->m_OnNew = std::move(Fn);
    m_Root->m_Children.Push(std::move(Child));

    return m_Root->m_Children.Back().Get();
}

Type* TypeDatabase::InternalRegister(const char* Name, const char* Parent, Type::OnNewSignature&& Fn)
{
    Type* Registered { GetType(m_Root.Get(), Name) };

    if (Registered != nullptr)
    {
        LS_ASSERT(Registered->m_Parent != nullptr);
        LS_ASSERT(Registered->m_Parent->m_Name == Parent);
        return Registered;
    }

    Type* ParentType { GetType(m_Root.Get(), Parent) };
    LS_ASSERTF(ParentType != nullptr, "Parent type (%s) does not exist.", Parent);

    UniquePtr<Type> Child { UniquePtr<Type>::New(Name) };
    Child->m_Parent = ParentType;
    Child->m_OnNew = std::move(Fn);
    ParentType->m_Children.Push(std::move(Child));

    return m_Root->m_Children.Back().Get();
}

Type* TypeDatabase::GetType(Type* Parent, const char* Name) const
{
    if (Parent->m_Name == Name)
    {
        return Parent;
    }

    for (const UniquePtr<Type>& Child : Parent->m_Children)
    {
        if (Child->m_Name == Name)
        {
            return Child.Get();
        }
    }

    for (const UniquePtr<Type>& Child : Parent->m_Children)
    {
        Type* Result = GetType(Child.Get(), Name);

        if (Result != nullptr)
        {
            return Result;
        }
    }

    return nullptr;
}

bool TypeDatabase::HasType(Type* Parent, const char* Name) const
{
    return GetType(Parent, Name) != nullptr;
}

void TypeDatabase::Clear()
{
    m_Root->m_Children.Clear();
}

}
}

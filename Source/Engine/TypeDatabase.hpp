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

#pragma once

#include "../Core/Memory/UniquePtr.hpp"
#include "Type.hpp"

#include <type_traits>

namespace LevelSketch
{
namespace Engine
{

class TypeDatabase final
{
public:
    static TypeDatabase& Instance();

    template<typename T>
    Type* Register(const char* Name)
    {
        return InternalRegister(Name, []() -> void* { return new T; });
    }

    template<typename T, typename U>
    Type* Register(const char* Name, const char* Parent)
    {
        static_assert(std::is_base_of_v<U, T>, "Type T is not a child of type U!");
        return InternalRegister(Name, Parent, []() -> void* { return new T; });
    }

    Type* GetType(const char* Name) const;
    bool HasType(const char* Name) const;
    bool Inherits(const char* Name, const char* Parent) const;
    void Clear();

private:
    TypeDatabase();

    Type* InternalRegister(const char* Name, Type::OnNewSignature&& Fn);
    Type* InternalRegister(const char* Name, const char* Parent, Type::OnNewSignature&& Fn);

    Type* GetType(Type* Parent, const char* Name) const;
    bool HasType(Type* Parent, const char* Name) const;

    UniquePtr<Type> m_Root { nullptr };
};

}
}

#ifndef DECLARE_BASE_TYPE
    #define DECLARE_BASE_TYPE(Type) LevelSketch::Engine::TypeDatabase::Instance().Register<Type>(#Type)
#else
    #error "DECLARE_BASE_TYPE has already been defined!"
#endif

#ifndef DECLARE_TYPE
    #define DECLARE_TYPE(Type, Parent) LevelSketch::Engine::TypeDatabase::Instance().Register<Type, Parent>(#Type, #Parent)
#else
    #error "DECLARE_TYPE has already been defined!"
#endif

#ifndef TYPE_EXISTS
    #define TYPE_EXISTS(Type) LevelSketch::Engine::TypeDatabase::Instance().HasType(#Type)
#else
    #error "TYPE_EXISTS has already been defined!"
#endif

#ifndef TYPE_INHERITS
    #define TYPE_INHERITS(Type, Parent) LevelSketch::Engine::TypeDatabase::Instance().Inherits(#Type, #Parent)
#else
    #error "TYPE_INHERITS has already been defined!"
#endif

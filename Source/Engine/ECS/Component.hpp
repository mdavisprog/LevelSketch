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

#include "../../Core/Types.hpp"

namespace LevelSketch
{
namespace Engine
{
namespace ECS
{

using ComponentID = u32;

static constexpr ComponentID InvalidComponentID { static_cast<ComponentID>(-1) };

class ComponentIDGenerator
{
private:
    static ComponentID s_Generator;

public:
    static void Reset();

private:
    template<typename T>
    friend class Component;

    template<typename T>
    static ComponentID Generate()
    {
        return s_Generator++;
    }
};

template<typename T>
class Component
{
public:
    static ComponentID ID()
    {
        return s_ID;
    }

    static ComponentID Register()
    {
        if (s_ID == InvalidComponentID)
        {
            s_ID = ComponentIDGenerator::Generate<T>();
        }

        return s_ID;
    }

private:
    static ComponentID s_ID;
};

template<typename T>
ComponentID Component<T>::s_ID { InvalidComponentID };

}
}
}

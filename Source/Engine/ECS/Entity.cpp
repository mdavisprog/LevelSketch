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

#include "Entity.hpp"

namespace LevelSketch
{
namespace Engine
{
namespace ECS
{

u32 EntityID::s_ID { 0 };

EntityID EntityID::Generate()
{
    EntityID Result {};
    Result.m_ID = (Result.m_ID & 0xFFFFFFFF00000000) | s_ID++;
    return Result;
}

void EntityID::Reset()
{
    s_ID = 0;
}

bool EntityID::operator==(const EntityID& Other) const
{
    return m_ID == Other.m_ID;
}

bool EntityID::operator!=(const EntityID& Other) const
{
    return m_ID != Other.m_ID;
}

u32 EntityID::ID() const
{
    return static_cast<u32>(m_ID & 0xFFFFFFFFFFFFFFFF);
}

}
}
}

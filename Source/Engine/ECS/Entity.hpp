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

#include "Archetype.hpp"

namespace LevelSketch
{
namespace Engine
{
namespace ECS
{

//
// EntityID
//
// A 64-bit integer where the low integer represents the unique ID and the high bits
// are reserveed for other information which will be defined later.
//

class EntityID
{
private:
    static u32 s_ID;

public:
    static EntityID Generate();
    static void Reset();

    bool operator==(const EntityID& Other) const;
    bool operator!=(const EntityID& Other) const;

    u32 ID() const;

private:
    u64 m_ID { 0 };
};

//
// EntityIDTraits
//
// Traits defined for hashing into a HashMap/HashSet.
//

struct EntityIDTraits : public Core::Traits::Base<EntityID>
{
    static u64 Hash(const EntityID& Value)
    {
        return Core::Traits::Base<u32>::Hash(Value.ID());
    }
};

//
// Entity
//
// A record that stores the EntityID, the archetype it is created from and the
// the row within the components array of the archetype.
//

struct Entity
{
    EntityID ID {};
    ArchetypeID Type {};
    u64 Row { 0 };
};

}
}
}

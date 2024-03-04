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

#include "../../Core/Containers/Array.hpp"
#include "../../Core/Containers/HashMap.hpp"
#include "../../Core/Containers/HashSet.hpp"
#include "../../Core/Traits.hpp"
#include "Component.hpp"
#include "ComponentPool.hpp"

namespace LevelSketch
{
namespace Engine
{
namespace ECS
{

using ArchetypeID = u32;
using ArchetypeKey = Array<ComponentID>;
using ArchetypeColumn = Array<ComponentPool>;

static constexpr ArchetypeID InvalidArchetypeID { static_cast<ArchetypeID>(-1) };

struct ArchetypeKeyTraits : Core::Traits::Base<ArchetypeKey>
{
    static u64 Hash(const ArchetypeKey& Key)
    {
        u64 Result { 0 };
        for (const ComponentID& Item : Key)
        {
            Result ^= Core::Traits::Base<ComponentID>::Hash(Item);
        }
        return Result;
    }
};

//
// Archetype
//
// An archetype is represented by a list of unique components. The World object keeps a list of
// archetypes using the unique componet list as the key. The archetype holds pools of components
// as columns and each element added to each component is a row that represents an entity.
//

struct Archetype
{
    ArchetypeID ID { InvalidArchetypeID };
    ArchetypeKey Key {};
    ArchetypeColumn Components {};

    bool operator==(const Archetype& Other) const
    {
        return ID == Other.ID;
    }

    bool operator!=(const Archetype& Other) const
    {
        return ID != Other.ID;
    }
};

//
// ArchetypeRecord
//
// Represents a set of archetypes that are registered to a component. The column within an archetype
// for a component can be found within the ComponentColumn field.
//
// The World object should hold the mapping for a ComponentID to an ArchetypeRecord.
//

struct ArchetypeRecord
{
    HashSet<ArchetypeID> Set {};
    HashMap<ArchetypeID, u64> ComponentColumn {};
};

}
}
}

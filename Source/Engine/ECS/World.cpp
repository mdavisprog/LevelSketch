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

#include "World.hpp"

namespace LevelSketch
{
namespace Engine
{
namespace ECS
{

static constexpr u64 InvalidRow { static_cast<u64>(-1) };

static bool HasComponents(const ArchetypeKey& Components, const ArchetypeKey& Archetype)
{
    for (const ComponentID& ID : Components)
    {
        if (!Archetype.Contains(ID))
        {
            return false;
        }
    }

    return true;
}

ArchetypeID World::s_ArchetypeID { 0 };

World::World()
{
}

u64 World::NumArchetypes() const
{
    return m_Archetypes.Size();
}

u64 World::NumEntities() const
{
    return m_Entities.Size();
}

World& World::Update(f32 DeltaTime)
{
    for (const System& System_ : m_Systems)
    {
        SystemData Data { *this, DeltaTime, GetArchetypes(System_.Components), System_.UserData };
        (System_.Callback)(Data);
    }

    return *this;
}

Archetype& World::GetOrAddArchetype(const ArchetypeKey& Key)
{
    if (!m_Archetypes.Contains(Key))
    {
        Archetype& Value { m_Archetypes[Key] };
        Value.ID = ++s_ArchetypeID;
        Value.Key = Key;
        Value.Components.Resize(Key.Size());
        m_ArchetypeKeys[Value.ID] = Key;

        for (u64 I = 0; I < Value.Key.Size(); I++)
        {
            const ComponentID ID { Value.Key[I] };
            ComponentPool& Pool { Value.Components[I] };
            Pool.SetElementSize(m_ComponentSizes[ID]);

            ArchetypeRecord& Record { m_ComponentRecords[ID] };
            Record.Set.Insert(Value.ID);
            Record.ComponentColumn[Value.ID] = I;
        }
    }

    return m_Archetypes[Key];
}

u64 World::AddRow(Archetype& Type) const
{
    u64 Result { InvalidRow };

    for (ComponentPool& Pool : Type.Components)
    {
        if (Result == InvalidRow)
        {
            Result = Pool.Size();
        }

        Pool.AddElement();
    }

    return Result;
}

EntityID World::NewEntity(const ArchetypeKey& Key)
{
    // TODO: Re-use expired entity IDs if they exist.

    const EntityID ID { EntityID::Generate() };

    Entity& Result { m_Entities[ID] };
    Result.ID = ID;

    Archetype& Type { GetOrAddArchetype(Key) };
    Result.Type = Type.ID;
    Result.Row = AddRow(Type);

    return ID;
}

u8* World::GetComponent(const EntityID& ID, const ComponentID& Component)
{
    Entity& Entity_ { m_Entities[ID] };
    const ArchetypeKey& Key { m_ArchetypeKeys[Entity_.Type] };
    Archetype& Type { m_Archetypes[Key] };

    ArchetypeRecord& Record { m_ComponentRecords[Component] };
    const u64 Column { Record.ComponentColumn[Type.ID] };
    return Type.Components[Column].GetElement(Entity_.Row);
}

Array<ArchetypeID> World::GetArchetypes(const ArchetypeKey& Components) const
{
    Array<ArchetypeID> Result {};

    for (const ArchetypeKey& Key : m_Archetypes.Keys())
    {
        if (HasComponents(Components, Key))
        {
            const Archetype& Type { m_Archetypes.At(Key) };
            Result.PushUnique(Type.ID);
        }
    }

    return Result;
}

const ComponentPool& World::GetComponents(const ArchetypeID& Type, const ComponentID& ID) const
{
    const ArchetypeKey& Key { m_ArchetypeKeys.At(Type) };
    const Archetype& Archetype_ { m_Archetypes.At(Key) };
    const ArchetypeRecord& Record { m_ComponentRecords.At(ID) };
    return Archetype_.Components[Record.ComponentColumn.At(Type)];
}

}
}
}

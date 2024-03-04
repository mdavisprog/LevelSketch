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
#include "../../Core/Sort/Insertion.hpp"
#include "Component.hpp"
#include "Entity.hpp"

namespace LevelSketch
{
namespace Engine
{
namespace ECS
{

class World
{
private:
    static ArchetypeID s_ArchetypeID;

public:
    World();

    template<typename... Components>
    EntityID NewEntity()
    {
        ArchetypeKey Key {};
        (GetArchetypeKey<Components>(Key), ...);
        Core::Sort::Insertion(Key);
        return NewEntity(Key);
    }

    template<typename T>
    T& GetComponent(const EntityID& ID)
    {
        return *reinterpret_cast<T*>(GetComponent(ID, Component<T>::ID()));
    }

    template<typename... Components>
    Array<ArchetypeID> GetArchetypes()
    {
        ArchetypeKey Key {};
        (GetArchetypeKey<Components>(Key), ...);
        return GetArchetypes(Key);
    }

    template<typename T>
    const ComponentPool& GetComponents(const ArchetypeID& Type) const
    {
        return GetComponents(Type, Component<T>::ID());
    }

    u64 NumArchetypes() const;
    u64 NumEntities() const;

private:
    template<typename T>
    void GetArchetypeKey(ArchetypeKey& Key)
    {
        const ComponentID ID { Component<T>::Register() };
        m_ComponentSizes[ID] = sizeof(T);
        Key.PushUnique(ID);
    }

    Archetype& GetOrAddArchetype(const ArchetypeKey& Key);
    u64 AddRow(Archetype& Type) const;
    EntityID NewEntity(const ArchetypeKey& Key);
    u8* GetComponent(const EntityID& ID, const ComponentID& Component);
    Array<ArchetypeID> GetArchetypes(const ArchetypeKey& Components) const;
    const ComponentPool& GetComponents(const ArchetypeID& Type, const ComponentID& ID) const;

    HashMap<EntityID, Entity, EntityIDTraits> m_Entities {};
    HashMap<ArchetypeKey, Archetype, ArchetypeKeyTraits> m_Archetypes {};
    HashMap<ArchetypeID, ArchetypeKey> m_ArchetypeKeys {};
    HashMap<ComponentID, u64> m_ComponentSizes {};
    HashMap<ComponentID, ArchetypeRecord> m_ComponentRecords {};
};

}
}
}

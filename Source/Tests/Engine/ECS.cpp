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

#include "../../Engine/ECS/Component.hpp"
#include "../../Engine/ECS/Entity.hpp"
#include "../../Engine/ECS/World.hpp"
#include "../TestSuite.hpp"
#include "../Utility.hpp"
#include "Engine.hpp"

namespace LevelSketch
{
namespace Tests
{
namespace Engine
{

namespace ECS = LevelSketch::Engine::ECS;

static bool EmptyWorld()
{
    ECS::World World {};
    VERIFY(World.NumEntities() == 0);
    return true;
}

static bool CreateEntity()
{
    struct A
    {
    };

    ECS::World World {};
    ECS::EntityID Entity1 { World.NewEntity<A>() };
    ECS::EntityID Entity2 { World.NewEntity<A>() };
    VERIFY(Entity1.ID() == 0);
    VERIFY(Entity2.ID() == 1);
    VERIFY(World.NumArchetypes() == 1);
    return true;
}

static bool ModifyComponent()
{
    struct A
    {
        i32 Value;
    };

    struct B
    {
        f32 Value;
    };

    ECS::World World {};
    ECS::EntityID Entity { World.NewEntity<A, B>() };
    World.GetComponent<A>(Entity).Value = 10;
    VERIFY(World.GetComponent<A>(Entity).Value == 10);
    World.GetComponent<B>(Entity).Value = 1.5f;
    VERIFY(World.GetComponent<B>(Entity).Value == 1.5f);
    return true;
}

static bool GetArchetypesWithComponents()
{
    struct A
    {
        bool Value;
    };

    struct B
    {
        f32 Value;
    };

    struct C
    {
        i32 Value;
    };

    ECS::World World {};
    ECS::EntityID Entity1 { World.NewEntity<A, C>() };
    ECS::EntityID Entity2 { World.NewEntity<B, C>() };
    ECS::EntityID Entity3 { World.NewEntity<C>() };
    VERIFY(World.NumEntities() == 3);

    World.GetComponent<A>(Entity1).Value = true;
    World.GetComponent<B>(Entity2).Value = 1.0f;
    World.GetComponent<C>(Entity1).Value = 1;
    World.GetComponent<C>(Entity2).Value = 2;
    World.GetComponent<C>(Entity3).Value = 5;

    const Array<ECS::ArchetypeID> AArchetypes { World.GetArchetypes<A>() };
    VERIFY(AArchetypes.Size() == 1);

    const Array<ECS::ArchetypeID> ACArchetypes { World.GetArchetypes<A, C>() };
    VERIFY(ACArchetypes.Size() == 1);

    const Array<ECS::ArchetypeID> CArchetypes { World.GetArchetypes<C>() };
    VERIFY(CArchetypes.Size() == 3);

    const Array<ECS::ArchetypeID> ABArchetypes { World.GetArchetypes<A, B>() };
    VERIFY(ABArchetypes.Size() == 0);

    return true;
}

static bool System()
{
    struct A
    {
        i32 Value;
    };

    bool Result { true };

    ECS::World World {};
    World.RegisterSystem<A>(
        [](ECS::World::SystemData& Data) -> void
        {
            bool* Result = static_cast<bool*>(Data.UserData);
            *Result &= Data.Types.Size() == 1;

            const ECS::ComponentPool& Pool { Data.TheWorld.GetComponents<A>(Data.Types[0]) };
            *Result &= Pool.Size() == 1;
            *Result &= Pool.Get<A>(0).Value == 5;
        },
        &Result);

    ECS::EntityID Entity { World.NewEntity<A>() };
    World.GetComponent<A>(Entity).Value = 5;
    World.Update(0.0f);
    return Result;
}

static bool DefaultComponentValues()
{
    struct A
    {
        i32 Value { 10 };
    };

    struct B
    {
        float Value { 1.5f };
    };

    ECS::World World {};
    ECS::EntityID Entity { World.NewEntity<A, B>() };

    VERIFY(World.GetComponent<A>(Entity).Value == 10);
    VERIFY(World.GetComponent<B>(Entity).Value == 1.5f);

    return true;
}

UniquePtr<TestSuite> ECSTests()
{
    return TestSuite::New("ECS",
        { TEST_CASE(EmptyWorld),
            TEST_CASE(CreateEntity),
            TEST_CASE(ModifyComponent),
            TEST_CASE(GetArchetypesWithComponents),
            TEST_CASE(System),
            TEST_CASE(DefaultComponentValues) });
}

}
}
}

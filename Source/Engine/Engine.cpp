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

#include "Engine.hpp"
#include "../Render/Renderer.hpp"
#include "Camera.hpp"
#include "Class.hpp"
#include "Components/Mesh.hpp"
#include "Components/Transform.hpp"
#include "ECS/World.hpp"

namespace LevelSketch
{
namespace Engine
{

Engine& Engine::Instance()
{
    static Engine Instance;
    return Instance;
}

bool Engine::Initialize()
{
    REGISTER_CLASS(Class);

    m_Camera = UniquePtr<Camera>::New();
    m_Camera->SetPosition({ 0.0f, 0.0f, -20.0f });

    m_World = UniquePtr<ECS::World>::New();

    return true;
}

void Engine::Shutdown()
{
}

void Engine::Update(float DeltaTime)
{
    m_World->Update(DeltaTime);
    m_Camera->Update(DeltaTime);
    Render::Renderer::Instance()->UpdateViewMatrix(m_Camera->ToViewMatrix());
}

void Engine::Render()
{
    const Array<ECS::ArchetypeID> Archetypes { m_World->GetArchetypes<Components::Transform, Components::Mesh>() };

    for (const ECS::ArchetypeID& ID : Archetypes)
    {
        const ECS::ComponentPool& MeshPool { m_World->GetComponents<Components::Mesh>(ID) };

        for (u64 I = 0; I < MeshPool.Size(); I++)
        {
            const Components::Mesh& Mesh { MeshPool.Get<Components::Mesh>(I) };

            Render::Renderer::Instance()->BindVertexBuffer(Mesh.VertexBuffer);
            Render::Renderer::Instance()->DrawIndexed(Mesh.Indices, 1, 0, 0, 0);
        }
    }
}

Camera* Engine::GetCamera() const
{
    return m_Camera.Get();
}

ECS::World* Engine::World() const
{
    return m_World.Get();
}

Engine::Engine()
{
}

}
}

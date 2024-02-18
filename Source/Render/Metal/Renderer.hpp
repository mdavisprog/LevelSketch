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
#include "../../Core/Math/Matrix.hpp"
#include "../../Core/Memory/UniquePtr.hpp"
#include "../Renderer.hpp"

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

class DepthStencil;
class Device;
class GraphicsPipeline;
class Texture;
class VertexBuffer;

struct Uniforms
{
    Matrix4f Model { Matrix4f::Identity };
    Matrix4f View { Matrix4f::Identity };
    Matrix4f Perspective { Matrix4f::Identity };
    Matrix4f Orthographic { Matrix4f::Identity };
};

class Renderer : public LevelSketch::Render::Renderer
{
public:
    Renderer();
    virtual ~Renderer();

    virtual bool Initialize() override;
    virtual bool Initialize(Platform::Window* Window) override;
    virtual void Shutdown() override;

    virtual u32 CreateTexture(const TextureDescription& Description) override;
    virtual bool BindTexture(u32 ID) override;

    virtual bool BeginRender(Platform::Window* Window, const Colorf& ClearColor) override;
    virtual void EndRender(Platform::Window* Window) override;
    virtual void SetViewportRect(const ViewportRect& Rect) override;
    virtual void SetScissor(const Recti& Rect) override;

    virtual u32 CreateGraphicsPipeline(const GraphicsPipelineDescription& Description) override;
    virtual bool BindGraphicsPipeline(u32 ID) override;

    virtual void DrawIndexed(u32 IndexCount,
        u32 InstanceCount,
        u32 StartIndex,
        u32 BaseVertex,
        u32 StartInstance) override;

    virtual u32 CreateVertexBuffer(const VertexBufferDescription& Description) override;
    virtual bool UploadVertexData(u32 ID, const VertexDataDescription& Description) override;
    virtual bool BindVertexBuffer(u32 ID) override;

    virtual void UpdateViewMatrix(const Matrix4f& View) override;

private:
    Texture* GetTexture(u32 ID) const;
    VertexBuffer* GetVertexBuffer(u32 ID) const;
    GraphicsPipeline* GetGraphicsPipeline(u32 ID) const;

    UniquePtr<Device> m_Device { nullptr };
    UniquePtr<DepthStencil> m_DepthStencil { nullptr };
    Array<UniquePtr<Texture>> m_Textures {};
    Array<UniquePtr<VertexBuffer>> m_VertexBuffers {};
    Array<UniquePtr<GraphicsPipeline>> m_GraphicsPipelines {};

    Uniforms m_Uniforms {};
};

}
}
}

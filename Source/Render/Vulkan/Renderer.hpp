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
#include "../../Core/Memory/UniquePtr.hpp"
#include "../Renderer.hpp"

#include <vulkan/vulkan.hpp>

#define FRAMES_IN_FLIGHT 2

namespace LevelSketch
{
namespace Render
{
namespace Vulkan
{

class CommandPool;
class DescriptorPool;
class Device;
class GraphicsPipeline;
class Sync;
class Texture;
class TexturePool;
class UniformBuffer;
class VertexBuffer;
class Viewport;

class Renderer : public LevelSketch::Render::Renderer
{
public:
    Renderer();
    virtual ~Renderer();

    virtual bool Initialize() override;
    virtual bool Initialize(Platform::Window* Window) override;
    virtual void Shutdown() override;

    virtual TextureHandle CreateTexture(const TextureDescription& Description) override;
    virtual bool BindTexture(const TextureHandle& Handle) override;

    virtual bool BeginRender(Platform::Window* Window, const Colorf& ClearColor) override;
    virtual void EndRender(Platform::Window* Window) override;
    virtual void SetViewportRect(const ViewportRect& Rect) override;
    virtual void SetScissor(const Recti& Rect) override;

    virtual GraphicsPipelineHandle CreateGraphicsPipeline(const GraphicsPipelineDescription& Description) override;
    virtual bool BindGraphicsPipeline(const GraphicsPipelineHandle& Handle) override;

    virtual void DrawIndexed(u32 IndexCount,
        u32 InstanceCount,
        u32 StartIndex,
        u32 BaseVertex,
        u32 StartInstance) override;

    virtual VertexBufferHandle CreateVertexBuffer(const VertexBufferDescription& Description) override;
    virtual bool UploadVertexData(const VertexBufferHandle& Handle, const VertexDataDescription& Description) override;
    virtual bool BindVertexBuffer(const VertexBufferHandle& Handle) override;

    virtual void UpdateViewMatrix(const Matrix4f& View) override;

private:
    bool GetRequiredExtensionProperties(const Array<VkExtensionProperties>& Properties, Array<const char*>& Ptrs) const;
    bool GetExistingLayers(const Array<const char*> Layers, Array<const char*>& Ptrs) const;
    VertexBuffer const* GetVertexBuffer(const VertexBufferHandle& Handle) const;
    GraphicsPipeline const* GetGraphicsPipeline(const GraphicsPipelineHandle& Handle) const;
    Viewport* GetViewport(Platform::Window* Window) const;

    VkInstance m_Instance { nullptr };
    UniquePtr<Device> m_Device { nullptr };
    UniquePtr<CommandPool> m_CommandPool { nullptr };
    UniquePtr<DescriptorPool> m_DescriptorPool { nullptr };
    UniquePtr<TexturePool> m_TexturePool { nullptr };
    Array<UniquePtr<Viewport>> m_Viewports {};
    Array<UniquePtr<Sync>> m_Syncs {};
    Array<UniquePtr<UniformBuffer>> m_Uniforms {};
    Array<UniquePtr<VertexBuffer>> m_VertexBuffers {};
    Array<UniquePtr<GraphicsPipeline>> m_GraphicsPipelines {};
    u64 m_FrameIndex { 0 };
    GraphicsPipeline* m_BoundPipeline { nullptr };
};

}
}
}

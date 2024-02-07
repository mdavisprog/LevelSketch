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
#include "../../Platform/Windows/Common.hpp"
#include "../Renderer.hpp"
#include "Texture.hpp"

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

class CommandQueue;
class DepthStencil;
class Device;
class GraphicsPipeline;
class SwapChain;
class VertexBuffer;
class Viewport;

#define FRAME_COUNT 2
#define MAX_DESCRIPTORS 1000

struct ConstantBufferData
{
public:
    Matrix4f Model { Matrix4f::Identity };
    Matrix4f View { Matrix4f::Identity };
    Matrix4f Perspective { Matrix4f::Identity };
    Matrix4f Orthographic { Matrix4f::Identity };
};

class Renderer : public LevelSketch::Render::Renderer
{
public:
    Renderer();

    virtual bool Initialize() override;
    virtual bool Initialize(Platform::Window* Window) override;
    virtual void Shutdown() override;

    virtual u32 LoadTexture(const void* Data, u32 Width, u32 Height, u8 BytesPerPixel = 4) override;
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
    bool LoadAssets();
    void WaitForPreviousFrame();
    bool ExecuteCommands();
    bool ResetCommands();
    u64 GetTextureOffset(u32 ID) const;
    Viewport* GetViewportFor(Platform::Window* Window) const;
    VertexBuffer* GetVertexBuffer(u32 ID) const;
    GraphicsPipeline* GetGraphicsPipeline(u32 ID) const;

    Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
    UINT64 m_FenceValue { 0 };
    HANDLE m_FenceEvent { nullptr };

    Array<Texture> m_Textures {};

    u32 m_DefaultTexture { 0 };

    Microsoft::WRL::ComPtr<ID3D12Resource> m_ConstantBuffer;
    u8* m_ConstantBufferAddress { nullptr };
    ConstantBufferData m_ConstantBufferData {};
    u64 m_ConstantBufferIndex { 0 };

    UniquePtr<Device> m_Device { nullptr };
    UniquePtr<DepthStencil> m_DepthStencil { nullptr };
    Array<UniquePtr<Viewport>> m_Viewports {};
    Array<UniquePtr<GraphicsPipeline>> m_GraphicsPipelines {};
    Array<UniquePtr<VertexBuffer>> m_VertexBuffers {};
};

}
}
}

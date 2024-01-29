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
#include "../../External/OctaneGUI/DrawCommand.h"
#include "../../Platform/Windows/Common.hpp"
#include "../Renderer.hpp"
#include "RenderBuffer.hpp"
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
class Device;
class SwapChain;

#define FRAME_COUNT 2
#define MAX_DESCRIPTORS 1000

struct ConstantBufferData
{
public:
    Matrix4f Model { Matrix4f::Identity };
    Matrix4f View { Matrix4f::Identity };
    Matrix4f Projection { Matrix4f::Identity };
    Matrix4f Orthographic { Matrix4f::Identity };
};

class Renderer : public LevelSketch::Render::Renderer
{
public:
    Renderer();

    virtual bool Initialize() override;
    virtual bool Initialize(Platform::Window* Window) override;
    virtual void Shutdown() override;
    virtual void Render(Platform::Window* Window) override;
    virtual u32 LoadTexture(const void* Data, u32 Width, u32 Height, u8 BytesPerPixel = 4) override;
    virtual void UploadGUIData(OctaneGUI::Window* Window, const OctaneGUI::VertexBuffer& Buffer) override;

private:
    bool LoadPipeline(Platform::Window* Window);
    bool LoadAssets(Platform::Window* Window);
    void WaitForPreviousFrame();
    bool ExecuteCommands();
    bool ResetCommands();
    u64 GetTextureOffset(u32 ID) const;

    UINT m_FrameIndex { 0 };
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_RenderTargets[FRAME_COUNT];
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineStateGUI;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList1> m_CommandList;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencil;
    UINT64 m_FenceValue { 0 };
    HANDLE m_FenceEvent { nullptr };

    RenderBuffer m_RenderBuffer {};
    RenderBuffer m_RenderBufferGUI {};
    Array<Texture> m_Textures {};
    Array<OctaneGUI::DrawCommand> m_GUICommands {};

    u32 m_WhiteTexture { 0 };
    u32 m_DefaultTexture { 0 };

    Microsoft::WRL::ComPtr<ID3D12Resource> m_ConstantBuffer;
    u8* m_ConstantBufferAddress { nullptr };
    ConstantBufferData m_ConstantBufferData {};
    u64 m_ConstantBufferIndex { 0 };

    UniquePtr<Device> m_Device { nullptr };
};

}
}
}

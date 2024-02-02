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
#include "../../Core/Math/Vector2.hpp"
#include "../../External/OctaneGUI/DrawCommand.h"
#include "RenderBuffer.hpp"
#include "Texture.hpp"

struct CGSize;

@class CAMetalLayer;

@protocol MTLCommandBuffer;
@protocol MTLCommandQueue;
@protocol MTLDepthStencilState;
@protocol MTLDevice;
@protocol MTLRenderPipelineState;
@protocol MTLTexture;

namespace OctaneGUI
{
class VertexBuffer;
}

namespace LevelSketch
{

namespace Platform
{
class Window;
}

namespace Render
{
namespace Metal
{

struct Uniforms
{
    Matrix4f Model { Matrix4f::Identity };
    Matrix4f View { Matrix4f::Identity };
    Matrix4f Perspective { Matrix4f::Identity };
    Matrix4f Orthographic { Matrix4f::Identity };
};

class RenderBridge
{
public:
    RenderBridge();

    bool Initialize(CAMetalLayer* Layer, Platform::Window* Window);
    void Render(CAMetalLayer* Layer, f64 ScaleFactor);

    u32 LoadTexture(const void* Data, u32 Width, u32 Height, u8 BytesPerPixel);
    void UploadGUIData(const OctaneGUI::VertexBuffer& Buffer);
    void UpdateViewMatrix(const Matrix4f& View);

private:
    // The size should already be scaled.
    RenderBridge& UpdateDepthBuffer(const CGSize& Size);
    id<MTLTexture> GetTexture(u32 ID) const;

    id<MTLDevice> m_Device { nullptr };
    id<MTLCommandQueue> m_CommandQueue { nullptr };
    id<MTLTexture> m_DepthBuffer { nullptr };
    id<MTLDepthStencilState> m_DepthStencil { nullptr };
    id<MTLDepthStencilState> m_DepthStencilGUI { nullptr };

    id<MTLRenderPipelineState> m_PipelineState { nullptr };
    id<MTLRenderPipelineState> m_PipelineStateGUI { nullptr };

    RenderBuffer m_RenderBuffer {};
    RenderBuffer m_RenderBufferGUI {};
    Array<OctaneGUI::DrawCommand> m_GUICommands {};

    Uniforms m_Uniforms {};
    Array<Texture> m_Textures {};
    id<MTLTexture> m_WhiteTexture { nullptr };
};

}
}
}

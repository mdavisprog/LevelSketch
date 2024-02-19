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

#include "../Core/Containers/String.hpp"
#include "../Core/Math/Forwards.hpp"
#include "../Core/Memory/UniquePtr.hpp"

namespace LevelSketch
{

namespace Platform
{
struct TimingData;
class Window;
}

namespace Render
{

struct GraphicsPipelineDescription;
struct TextureDescription;
struct VertexBufferDescription;
struct VertexDataDescription;
struct ViewportRect;

class Renderer
{
private:
    // Must be implemented by API.
    static UniquePtr<Renderer> CreateInstance();

public:
    struct DriverSummary
    {
    public:
        String Vendor {};
        String Renderer {};
        String Version {};
        String ShadingLanguageVersion {};
    };

    static const UniquePtr<Renderer>& Instance();
    static String ShadersDirectory();
    static String ShaderPath(const char* FileName);

    virtual ~Renderer()
    {
    }

    virtual bool Initialize() = 0;
    virtual bool Initialize(Platform::Window* Window) = 0;
    virtual void Shutdown() = 0;

    virtual u32 CreateTexture(const TextureDescription& Description) = 0;
    virtual bool BindTexture(u32 ID) = 0;

    virtual bool BeginRender(Platform::Window* Window, const Colorf& ClearColor) = 0;
    virtual void EndRender(Platform::Window* Window) = 0;
    virtual void SetViewportRect(const ViewportRect& Rect) = 0;
    virtual void SetScissor(const Recti& Rect) = 0;

    virtual u32 CreateGraphicsPipeline(const GraphicsPipelineDescription& Description) = 0;
    virtual bool BindGraphicsPipeline(u32 ID) = 0;

    virtual void DrawIndexed(u32 IndexCount, u32 InstanceCount, u32 StartIndex, u32 BaseVertex, u32 StartInstance) = 0;

    virtual u32 CreateVertexBuffer(const VertexBufferDescription& Description) = 0;
    virtual bool UploadVertexData(u32 ID, const VertexDataDescription& Description) = 0;
    virtual bool BindVertexBuffer(u32 ID) = 0;

    // Temporary
    virtual void UpdateViewMatrix(const Matrix4f& View);

    const DriverSummary& Summary() const;
    Platform::TimingData TimingData() const;

protected:
    DriverSummary& SummaryMut();

private:
    DriverSummary m_DriverSummary {};
};

}
}

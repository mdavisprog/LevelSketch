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

namespace OctaneGUI
{
class VertexBuffer;
class Window;
}

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

class Renderer
{
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
    virtual void Render(Platform::Window* Window) = 0;
    virtual u32 LoadTexture(const void* Data, u32 Width, u32 Height, u8 BytesPerPixel = 4);
    virtual u32 CreateGraphicsPipeline(const GraphicsPipelineDescription& Description) = 0;

    // Temporary
    virtual void UploadGUIData(OctaneGUI::Window* Window, const OctaneGUI::VertexBuffer& Buffer);
    virtual void UpdateViewMatrix(const Matrix4f& View);

    bool Initialized() const;
    const DriverSummary& Summary() const;
    Platform::TimingData TimingData() const;

protected:
    void SetInitialized(bool Initialized);
    DriverSummary& SummaryMut();

private:
    bool m_Initialized { false };
    DriverSummary m_DriverSummary {};
};

}
}

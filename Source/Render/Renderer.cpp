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

#include "Renderer.hpp"
#include "../Core/Memory/UniquePtr.hpp"
#include "../Platform/FileSystem.hpp"
#include "../Platform/Platform.hpp"
#include "../Platform/Window.hpp"

#if defined(RENDER_DIRECTX)
#include "DirectX/Renderer.hpp"
#elif defined(RENDER_METAL)
#include "Metal/Renderer.hpp"
#elif defined(RENDER_VULKAN)
#include "Vulkan/Renderer.hpp"
#elif defined(RENDER_OPENGL)
#if defined(PLATFORM_SDL2)
#include "OpenGL/SDL2Renderer.hpp"
#else
#include "OpenGL/Renderer.hpp"
#endif
#else
#error "Renderer is not supported!"
#endif

namespace LevelSketch
{
namespace Render
{

const UniquePtr<Renderer>& Renderer::Instance()
{
    static UniquePtr<Renderer> Instance
    {
#if defined(RENDER_DIRECTX)
        UniquePtr<DirectX::Renderer>::New()
#elif defined(RENDER_METAL)
        UniquePtr<Metal::Renderer>::New()
#elif defined(RENDER_VULKAN)
        UniquePtr<Vulkan::Renderer>::New()
#elif defined(RENDER_OPENGL)
#if defined(PLATFORM_SDL2)
        UniquePtr<OpenGL::SDL2Renderer>::New()
#else
        UniquePtr<OpenGL::Renderer>::New()
#endif
#endif
    };

    return Instance;
}

String Renderer::ShaderPath(const char* FileName)
{
    return Platform::FileSystem::CombinePaths(ShadersDirectory(), FileName);
}

u32 Renderer::LoadTexture(const void*, u32, u32, u8)
{
    return 0;
}

void Renderer::UpdateViewMatrix(const Matrix4f&)
{
}

const Renderer::DriverSummary& Renderer::Summary() const
{
    return m_DriverSummary;
}

Renderer::DriverSummary& Renderer::SummaryMut()
{
    return m_DriverSummary;
}

Platform::TimingData Renderer::TimingData() const
{
    return Platform::Platform::Instance()->GetTimingData();
}

}
}

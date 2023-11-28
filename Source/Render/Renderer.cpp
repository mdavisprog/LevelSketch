/**

MIT License

Copyright (c) 2023 Mitchell Davis <mdavisprog@gmail.com>

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

#if defined(RENDER_DIRECTX)
    #include "DirectX/Renderer.hpp"
#elif defined(RENDER_METAL)
    #include "Metal/Renderer.hpp"
#elif defined(RENDER_OPENGL)
    #include "OpenGL/Renderer.hpp"
#else
    #error "Renderer is not supported!"
#endif

namespace LevelSketch
{
namespace Render
{

const Core::Memory::UniquePtr<Renderer>& Renderer::Instance()
{
    static Core::Memory::UniquePtr<Renderer> Instance
    {
#if defined(RENDER_DIRECTX)
        Core::Memory::UniquePtr<DirectX::Renderer>::New()
#elif defined(RENDER_METAL)
        Core::Memory::UniquePtr<Metal::Renderer>::New()
#elif defined(RENDER_OPENGL)
        Core::Memory::UniquePtr<OpenGL::Renderer>::New()
#endif
    };

    return Instance;
}

Renderer& Renderer::SetWindow(Platform::Window* Window)
{
    m_Window = Window;
    return *this;
}

Platform::Window* Renderer::Window() const
{
    return m_Window;
}

}
}

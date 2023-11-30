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

#include <GL/gl.h>

namespace LevelSketch
{
namespace Render
{
namespace OpenGL
{

Renderer::Renderer()
    : LevelSketch::Render::Renderer()
{
}

bool Renderer::Initialize()
{
    return true;
}

bool Renderer::Initialize(Platform::Window*)
{
    if (!Initialized())
    {
        // Currently, context creation is only handled by the SDL2Renderer. If using other
        // platforms like glfw or native OS, will need to provide context creation in order
        // for all gl* calls to work.

        SummaryMut().Vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        SummaryMut().Renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        SummaryMut().Version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
        SummaryMut().ShadingLanguageVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

        SetInitialized(true);
    }

    return true;
}

void Renderer::Shutdown()
{
}

void Renderer::Render(Platform::Window*)
{
}

}
}
}

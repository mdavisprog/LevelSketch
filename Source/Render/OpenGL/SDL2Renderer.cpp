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

#include "SDL2Renderer.hpp"
#include "../../Platform/Window.hpp"
#include "SDL2/SDL.h"

namespace LevelSketch
{
namespace Render
{
namespace OpenGL
{

SDL2Renderer::SDL2Renderer()
    : Renderer()
{
}

bool SDL2Renderer::Initialize()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    return Renderer::Initialize();
}

bool SDL2Renderer::Initialize(Platform::Window* Window)
{
    if (Window == nullptr)
    {
        return false;
    }

    if (m_Context == nullptr)
    {
        SDL_Window* Handle { static_cast<SDL_Window*>(Window->Handle()) };

        if (Handle == nullptr)
        {
            return false;
        }

        m_Context = SDL_GL_CreateContext(Handle);
        if (m_Context == nullptr)
        {
            return false;
        }

        int Major, Minor;
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &Major);
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &Minor);

        printf("SDL OpenGL Context Version: %d.%d\n", Major, Minor);
    }

    return Renderer::Initialize(Window);
}

void SDL2Renderer::Shutdown()
{
    if (m_Context != nullptr)
    {
        SDL_GL_DeleteContext(m_Context);
        m_Context = nullptr;
    }
}

}
}
}

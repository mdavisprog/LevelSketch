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

#include "Platform.hpp"
#include "SDL2/SDL.h"
#include "Window.hpp"

namespace LevelSketch
{
namespace Platform
{
namespace SDL2
{

Platform::Platform()
    : LevelSketch::Platform::Platform()
{
}

bool Platform::Initialize()
{
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
    {
        printf("Failed to initialize SDL2: %s\n", SDL_GetError());
        return false;
    }

    SDL_version Version {};
    SDL_GetVersion(&Version);
    printf("Using SDL version %d.%d.%d\n", Version.major, Version.minor, Version.patch);

    return true;
}

void Platform::Shutdown()
{
    SDL_Quit();
}

const char* Platform::Name() const
{
    return "SDL2";
}

LevelSketch::Platform::Window* Platform::NewWindow(const char* Title, int X, int Y, int Width, int Height)
{
    Window* Result { new Window() };

    if (!Result->Create(Title, X, Y, Width, Height))
    {
        delete Result;
        Result = nullptr;
    }

    return Result;
}

void Platform::CloseWindow(LevelSketch::Platform::Window* Window)
{
    if (Window == nullptr)
    {
        return;
    }

    Window->Close();
}

}
}
}

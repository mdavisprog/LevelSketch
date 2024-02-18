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

#include "../Mouse.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "../Window.hpp"
#include "SDL2/SDL.h"

namespace LevelSketch
{
namespace Platform
{

void Mouse::Show()
{
    SDL_ShowCursor(SDL_ENABLE);
}

void Mouse::Hide()
{
    SDL_ShowCursor(SDL_DISABLE);
}

bool Mouse::IsVisible()
{
    return SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE;
}

void Mouse::SetPosition(const Vector2i& Position)
{
    SDL_WarpMouseGlobal(Position.X, Position.Y);
}

void Mouse::SetPosition(Window* Target, const Vector2i& Position)
{
    SDL_Window* Handle { reinterpret_cast<SDL_Window*>(Target->Handle()) };
    SDL_WarpMouseInWindow(Handle, Position.X, Position.Y);
}

}
}

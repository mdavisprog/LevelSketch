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

#include "Extensions.hpp"
#include "../../Core/Assert.hpp"

#include <GL/glx.h>

namespace LevelSketch
{
namespace Render
{
namespace OpenGL
{

PFNGLCREATESHADERPROC glCreateShader { nullptr };
PFNGLDELETESHADERPROC glDeleteShader { nullptr };
PFNGLSHADERSOURCEPROC glShaderSource { nullptr };
PFNGLCOMPILESHADERPROC glCompileShader { nullptr };
PFNGLGETSHADERIVPROC glGetShaderiv { nullptr };
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog { nullptr };
PFNGLCREATEPROGRAMPROC glCreateProgram { nullptr };
PFNGLDELETEPROGRAMPROC glDeleteProgram { nullptr };
PFNGLATTACHSHADERPROC glAttachShader { nullptr };
PFNGLDETACHSHADERPROC glDetachShader { nullptr };
PFNGLLINKPROGRAMPROC glLinkProgram { nullptr };
PFNGLGETPROGRAMIVPROC glGetProgramiv { nullptr };
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog { nullptr };

template<typename T>
T LoadProcedure(const char* Name)
{
    T Result { reinterpret_cast<T>(glXGetProcAddress(reinterpret_cast<const GLubyte*>(Name))) };
    LS_ASSERTF(Result != nullptr, "Failed to load OpenGL extension procedure '%s'!", Name);
    return Result;
}

bool LoadExtensions()
{
    glCreateShader = LoadProcedure<PFNGLCREATESHADERPROC>("glCreateShader");
    glDeleteShader = LoadProcedure<PFNGLDELETESHADERPROC>("glDeleteShader");
    glShaderSource = LoadProcedure<PFNGLSHADERSOURCEPROC>("glShaderSource");
    glCompileShader = LoadProcedure<PFNGLCOMPILESHADERPROC>("glCompileShader");
    glGetShaderiv = LoadProcedure<PFNGLGETSHADERIVPROC>("glGetShaderiv");
    glGetShaderInfoLog = LoadProcedure<PFNGLGETSHADERINFOLOGPROC>("glGetShaderInfoLog");
    glCreateProgram = LoadProcedure<PFNGLCREATEPROGRAMPROC>("glCreateProgram");
    glDeleteProgram = LoadProcedure<PFNGLDELETEPROGRAMPROC>("glDeleteProgram");
    glAttachShader = LoadProcedure<PFNGLATTACHSHADERPROC>("glAttachShader");
    glDetachShader = LoadProcedure<PFNGLDETACHSHADERPROC>("glDetachShader");
    glLinkProgram = LoadProcedure<PFNGLLINKPROGRAMPROC>("glLinkProgram");
    glGetProgramiv = LoadProcedure<PFNGLGETPROGRAMIVPROC>("glGetProgramiv");
    glGetProgramInfoLog = LoadProcedure<PFNGLGETPROGRAMINFOLOGPROC>("glGetProgramInfoLog");

    return true;
}

}
}
}

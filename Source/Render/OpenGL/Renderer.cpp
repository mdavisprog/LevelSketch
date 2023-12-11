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
#include "../../Core/Containers/Array.hpp"
#include "Extensions.hpp"

#include <cstdio>
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

        LoadExtensions();
        CompileShaders();

        SetInitialized(true);
    }

    return true;
}

void Renderer::Shutdown()
{
    if (m_Program != 0)
    {
        glDeleteProgram(m_Program);
        m_Program = 0;
    }
}

void Renderer::Render(Platform::Window*)
{
}

u32 Renderer::LoadTexture(const void*, u32, u32, u8)
{
    return 1;
}

bool Renderer::CompileShaders()
{
    const GLchar* Version = "#version 450\n";
    const GLchar* VertexShader =
        "uniform mat4 Projection;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Fragment_UV;\n"
        "out vec4 Fragment_Color;\n"
        "void main()\n"
        "{\n"
        "	Fragment_UV = UV;\n"
        "	Fragment_Color = Color;\n"
        "	gl_Position = Projection * vec4(Position.xy, 0, 1);\n"
        "}\n";

    const GLchar* FragmentShader =
        "uniform sampler2D Texture;\n"
        "in vec2 Fragment_UV;\n"
        "in vec4 Fragment_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "	Out_Color = Fragment_Color * texture(Texture, Fragment_UV.st);\n"
        "}\n";

    const GLchar* VertexShaderInfo[2] = { Version, VertexShader };
    GLuint VertexID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexID, 2, VertexShaderInfo, nullptr);
    LS_ASSERT(CompileShader(VertexID));

    const GLchar* FragmentShaderInfo[2] = { Version, FragmentShader };
    GLuint FragmentID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentID, 2, FragmentShaderInfo, nullptr);
    LS_ASSERT(CompileShader(FragmentID));

    m_Program = glCreateProgram();
    glAttachShader(m_Program, VertexID);
    glAttachShader(m_Program, FragmentID);
    LS_ASSERT(LinkProgram(m_Program));

    glDetachShader(m_Program, VertexID);
    glDetachShader(m_Program, FragmentID);
    glDeleteShader(VertexID);
    glDeleteShader(FragmentID);

    return true;
}

bool Renderer::CompileShader(u32 Shader) const
{
    if (Shader == 0)
    {
        return false;
    }

    glCompileShader(Shader);

    GLint CompileStatus;
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &CompileStatus);

    if ((GLboolean)CompileStatus == GL_FALSE)
    {
        GLint Length;
        glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &Length);

        Array<GLchar> Buffer;
        Buffer.Resize(Length);

        GLsizei Size;
        glGetShaderInfoLog(Shader, (GLsizei)Buffer.Size(), &Size, Buffer.Data());

        printf("Failed to compile shader! Error:\n%s\n", Buffer.Data());
        return false;
    }

    return true;
}

bool Renderer::LinkProgram(u32 Program) const
{
    if (Program == 0)
    {
        return false;
    }

    glLinkProgram(Program);

    GLint LinkStatus { 0 };
    glGetProgramiv(Program, GL_LINK_STATUS, &LinkStatus);
    if ((GLboolean)LinkStatus == GL_FALSE)
    {
        GLint Length;
        glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &Length);

        Array<GLchar> Buffer;
        Buffer.Resize(Length);

        GLsizei Size;
        glGetProgramInfoLog(Program, (GLsizei)Buffer.Size(), &Size, Buffer.Data());

        printf("Failed to link program. Error:\n%s\n", Buffer.Data());
        return false;
    }

    return true;
}

}
}
}

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

#include "Shader.hpp"

#include <d3dcompiler.h>

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

Shader::Shader()
{
}

Shader& Shader::SetName(const char* Name)
{
    m_Name = Name;
    return *this;
}

Shader& Shader::SetSource(const char* Source)
{
    m_Source = Source;
    return *this;
}

Shader& Shader::SetEntryPoint(const char* EntryPoint)
{
    m_EntryPoint = EntryPoint;
    return *this;
}

Shader& Shader::SetTarget(const char* Target)
{
    m_Target = Target;
    return *this;
}

Shader& Shader::SetCompileFlags(u32 CompileFlags)
{
    m_CompileFlags = CompileFlags;
    return *this;
}

Shader& Shader::AddInputElement(D3D12_INPUT_ELEMENT_DESC Description)
{
    m_InputElements.Push(Description);
    return *this;
}

Shader& Shader::ClearInputElements()
{
    m_InputElements.Clear();
    return *this;
}

bool Shader::Compile()
{
    m_Blob = nullptr;
    m_Errors = nullptr;

    HRESULT Result = D3DCompile(
        m_Source.Data(),
        m_Source.Length(),
        m_Name.Data(),
        nullptr,
        nullptr,
        m_EntryPoint.Data(),
        m_Target.Data(),
        m_CompileFlags,
        0,
        &m_Blob,
        &m_Errors
    );

    return Result == S_OK;
}

}
}
}
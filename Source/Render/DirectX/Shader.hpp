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

#include "../../Core/Containers/String.hpp"

#include <d3d12.h>
#include <wrl/client.h>

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

class Shader
{
public:
    static Shader LoadFromFile(const char* FileName);

    Shader();

    Shader& SetName(const char* Name);
    Shader& SetSource(const char* Source);
    Shader& SetEntryPoint(const char* EntryPoint);
    Shader& SetTarget(const char* Target);
    Shader& SetCompileFlags(u32 CompileFlags);

    Shader& AddInputElement(D3D12_INPUT_ELEMENT_DESC Description);
    Shader& ClearInputElements();
    D3D12_INPUT_ELEMENT_DESC* InputElements()
    {
        return m_InputElements.Data();
    }
    u32 NumInputElements() const
    {
        return static_cast<u32>(m_InputElements.Size());
    }

    const Microsoft::WRL::ComPtr<ID3DBlob>& Blob() const
    {
        return m_Blob;
    }
    const Microsoft::WRL::ComPtr<ID3DBlob>& Errors() const
    {
        return m_Errors;
    }

    bool Compile();

private:
    String m_Name {};
    String m_Source {};
    String m_EntryPoint {};
    String m_Target {};
    Array<D3D12_INPUT_ELEMENT_DESC> m_InputElements {};

    u32 m_CompileFlags { 0 };

    Microsoft::WRL::ComPtr<ID3DBlob> m_Blob { nullptr };
    Microsoft::WRL::ComPtr<ID3DBlob> m_Errors { nullptr };
};

}
}
}

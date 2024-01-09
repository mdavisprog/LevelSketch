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

#include "Shader.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/FileSystem.hpp"

#import <MetalKit/MetalKit.h>

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

Shader::Shader()
{
}

bool Shader::LoadFile(id<MTLDevice> Device, const char* Path)
{
    if (IsLoaded())
    {
        return true;
    }

    const String Contents { Platform::FileSystem::ReadContents(Path) };
    if (!Load(Device, Contents.Data()))
    {
        Core::Console::Error("Failed to compile shader file: %s.", Path);
        return false;
    }

    return true;
}

bool Shader::Load(id<MTLDevice> Device, const char* Source)
{
    if (IsLoaded())
    {
        return true;
    }

    NSError* Error { nullptr };
    m_Shaders = [Device newLibraryWithSource:[NSString stringWithUTF8String:Source]
                                     options:nil
                                       error:&Error];
    
    if (m_Shaders == nullptr)
    {
        Core::Console::Error("Failed to compile shader.\nError: %s", [[Error localizedDescription] UTF8String]);
        return false;
    }

    return true;
}

bool Shader::IsLoaded() const
{
    return m_Shaders != nullptr;
}

bool Shader::LoadVertex(const char* Name)
{
    if (m_Vertex == nullptr)
    {
        m_Vertex = LoadFunction(Name);
    }

    return m_Vertex != nullptr;
}

bool Shader::LoadPixel(const char* Name)
{
    if (m_Pixel == nullptr)
    {
        m_Pixel = LoadFunction(Name);
    }

    return m_Pixel != nullptr;
}

id<MTLFunction> Shader::Vertex() const
{
    return m_Vertex;
}

id<MTLFunction> Shader::Pixel() const
{
    return m_Pixel;
}

id<MTLFunction> Shader::LoadFunction(const char* Name) const
{
    if (m_Shaders == nullptr)
    {
        return nullptr;
    }

    MTLFunctionConstantValues* ConstantValues { [MTLFunctionConstantValues new] };
    NSError* Error { nullptr };
    id<MTLFunction> Result = [m_Shaders newFunctionWithName:[NSString stringWithUTF8String:Name]
                                             constantValues:ConstantValues
                                                      error:&Error];

    if (Result == nullptr)
    {
        Core::Console::Error("Failed to load function '%s'.\nError: %s", Name, [[Error localizedDescription] UTF8String]);
    }

    return Result;
}

}
}
}

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
#include "Device.hpp"

#import <Metal/Metal.h>

namespace LevelSketch
{
namespace Render
{
namespace Metal
{

Shader::Shader()
{
}

bool Shader::LoadFile(Device const* Device_, const char* Path)
{
    const String Contents { Platform::FileSystem::ReadContents(Path) };

    if (Contents.IsEmpty())
    {
        Core::Console::Error("No shader contents loaded for path '%s'.", Path);
        return false;
    }

    if (!Load(Device_, Contents.Data()))
    {
        return false;
    }

    return true;
}

bool Shader::Load(Device const* Device_, const char* Source)
{
    NSError* Error { nullptr };
    m_Library = [Device_->Get() newLibraryWithSource:[NSString stringWithUTF8String:Source] options:nil error:&Error];

    if (m_Library == nullptr)
    {
        Core::Console::Error("Failed to compile shader. Error: %s", [[Error localizedDescription] UTF8String]);
        return false;
    }

    return true;
}

id<MTLFunction> Shader::LoadFunction(const char* Name) const
{
    if (m_Library == nullptr)
    {
        return nullptr;
    }

    MTLFunctionConstantValues* ConstantValues { [MTLFunctionConstantValues new] };
    NSError* Error { nullptr };
    id<MTLFunction> Result = [m_Library newFunctionWithName:[NSString stringWithUTF8String:Name]
                                             constantValues:ConstantValues
                                                      error:&Error];

    if (Result == nullptr)
    {
        Core::Console::Error("Failed to load function '%s'. Error: %s",
            Name,
            [[Error localizedDescription] UTF8String]);
    }

    return Result;
}

}
}
}

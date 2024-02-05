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

#include "../FileSystem.hpp"
#include "../../Core/Containers/String.hpp"
#include "Common.hpp"

#include <direct.h>

namespace LevelSketch
{
namespace Platform
{

String FileSystem::ApplicationPath()
{
    WString Path;
    Path.Resize(255);

    DWORD Success { 0 };
    while (true)
    {
        Success = GetModuleFileNameW(nullptr, Path.Data(), static_cast<DWORD>(Path.Size()));

        if (Success == ERROR_INSUFFICIENT_BUFFER)
        {
            Path.Resize(Path.Capacity() * 2);
        }
        else
        {
            Path[Success] = 0;
            break;
        }
    }

    return Core::Containers::ToString(Path);
}

String FileSystem::ContentDirectory()
{
    return CombinePaths(ApplicationDirectory(), "Content");
}

void FileSystem::SetWorkingDirectory(const String& Path)
{
    _chdir(Path.Data());
}

}
}

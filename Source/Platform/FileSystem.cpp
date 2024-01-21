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

#include "FileSystem.hpp"
#include "../Core/Containers/String.hpp"
#include "../Core/Defines.hpp"
#include "../Core/Math/Math.hpp"

#include <fstream>
#include <string>

#if defined(WINDOWS)
    #define PATH_SEPARATOR "\\"
#else
    #define PATH_SEPARATOR "/"
#endif

namespace LevelSketch
{
namespace Platform
{

String FileSystem::ApplicationDirectory()
{
    return GetDirectory(ApplicationPath());
}

String FileSystem::GetDirectory(const String& Path)
{
    const u64 Forward { Path.RFind('/') };
    const u64 Backward { Path.RFind('\\') };

    u64 Pos { Forward };
    if (Forward == String::NPOS || (Backward != String::NPOS && Backward > Forward))
    {
        Pos = Backward;
    }

    if (Pos == String::NPOS)
    {
        return Path;
    }

    if (Pos == Path.Length())
    {
        return Path;
    }

    return Path.Sub(0, Pos);
}

String FileSystem::CombinePaths(const String& A, const String& B)
{
    return A + PATH_SEPARATOR + B;
}

String FileSystem::ReadContents(const String& Path)
{
    String Result;

    std::fstream Stream;
    Stream.open(Path.Data(), std::ios_base::in);
    if (!Stream.is_open())
    {
        return Result;
    }

    std::string Buffer;
    while (Stream.good())
    {
        std::getline(Stream, Buffer);
        Result += Buffer.c_str();
        Result += "\n";
    }
    Stream.close();

    return Result;
}

FileSystem::FileSystem()
{
}

}
}

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

#include "String.hpp"

#include <cwchar>

namespace LevelSketch
{
namespace Core
{
namespace Containers
{

// TODO: Look into using the secure API or use codecvt
#if defined(MSVC)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

String ToString(const WString& Value)
{
    String Result;

    const wchar_t* Data { Value.Data() };
    std::mbstate_t State {};
    size_t Length { std::wcsrtombs(nullptr, &Data, (size_t)Value.Size(), &State) };

    if (Length == static_cast<std::size_t>(-1) || Length == 0)
    {
        return Result;
    }

    Result.Resize(Length + 1);
    std::wcsrtombs(Result.Data(), &Data, (size_t)Value.Size(), &State);
    Result[Length] = 0;

    return Result;
}

WString ToWString(const String& Value)
{
    WString Result;

    const char* Data { Value.Data() };
    std::mbstate_t State {};
    size_t Length = std::mbsrtowcs(nullptr, &Data, (size_t)Value.Size(), &State);

    if (Length == static_cast<std::size_t>(-1) || Length == 0)
    {
        return Result;
    }

    Result.Resize(Length + 1);
    std::mbsrtowcs(Result.Data(), &Data, (size_t)Value.Size(), &State);
    Result[Length] = 0;

    return Result;
}

#if defined(MSVC)
#pragma warning(pop)
#endif

}
}
}

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

#include "Errors.hpp"
#include "../../Core/Console.hpp"
#include "Common.hpp"

namespace LevelSketch
{
namespace Platform
{
namespace Windows
{
namespace Errors
{

String ToString(i32 Error)
{
    TCHAR* Message { nullptr };
    const DWORD Flags { FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM };
    DWORD Length { FormatMessageW(Flags,
        nullptr,
        Error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<TCHAR*>(&Message),
        0,
        nullptr) };

    String Result {};
    if (Length == 0)
    {
        Core::Console::Error("Failed to format error message from error code (%d). Current error code(%d)",
            Error,
            GetLastError());
    }
    else
    {
        Result = Core::Containers::ToString(Message);
        LocalFree(Message);
    }

    return Result;
}

}
}
}
}

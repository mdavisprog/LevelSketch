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

#pragma once

#if defined(SHIPPING)
    #define LS_ASSERT(Condition)
    #define LS_ASSERTFEMPTY(Condition, Format, ...)
    #define LS_ASSERTF(Condition, Format, ...) LSASSERTFEMPTY(Condition, Format, NULL, ##__VA_ARGS__)
#else

#include "Compiler.hpp"
#include "Types.hpp"

#include <climits>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace LevelSketch
{
namespace Core
{

#if defined(MSVC)
    #pragma warning(push)
    #pragma warning(disable : 4505)
#endif

static void Assertion(const char* File, u32 Line, bool Condition, const char* Format, ...)
{
    if (Condition)
    {
        return;
    }

    va_list List;
    va_start(List, Format);

    std::string Buffer;
    Buffer.resize(SHRT_MAX);
    vsnprintf(Buffer.data(), Buffer.size(), Format, List);

    va_end(List);

    printf("%s\n", Buffer.data());
    printf("Assertion failed at %s:%u\n", File, Line);

    std::abort();
}

#if defined(MSVC)
    #pragma warning(pop)
#endif

}
}

#if defined(CLANG)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

#define LS_ASSERT(Condition) LevelSketch::Core::Assertion(__FILE__, __LINE__, Condition, #Condition)
#define LS_ASSERTF(Condition, Format, ...) LevelSketch::Core::Assertion(__FILE__, __LINE__, Condition, Format, ##__VA_ARGS__)

#if defined(CLANG)
    #pragma clang diagnostic pop
#endif

#endif // SHIPPING

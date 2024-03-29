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

#if __clang__
#ifndef CLANG
#define CLANG
#endif
#elif _MSC_VER
#ifndef MSVC
#define MSVC
#endif
#elif __GNUC__ || __GNUG__
#ifndef GCC
#define GCC
#endif
#endif

#ifndef PRAGMA_STRINGIFY
#define PRAGMA_STRINGIFY(X) _Pragma(#X)
#endif

#if __clang__
#define PRAGMA_WARNING(Warning) PRAGMA_STRINGIFY(clang diagnostic ignored #Warning)

#define PUSH_DISABLE_WARNING(Warning) _Pragma("clang diagnostic push") PRAGMA_WARNING(Warning)

#define POP_DISABLE_WARNING _Pragma("clang diagnostic pop")

#else
#define PUSH_DISABLE_WARNING(Warning)
#define POP_DISABLE_WARNING
#endif

#if defined(GCC)
#define MAYBE_UNUSED [[gnu::unused]]
#else
#define MAYBE_UNUSED
#endif

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

#include "../Containers/Array.hpp"

namespace LevelSketch
{
namespace Core
{
namespace Sort
{

template<typename T>
void Insertion(Array<T>& Value, u64 Start, u64 End)
{
    for (u64 I = Start + 1; I <= End; I++)
    {
        for (u64 J = I; J > 0; J--)
        {
            const bool ShouldSwap { Value[J] < Value[J - 1] };

            if (ShouldSwap)
            {
                T Temp { Value[J] };
                Value[J] = Value[J - 1];
                Value[J - 1] = Temp;
            }
        }
    }
}

template<typename T>
void Insertion(Array<T>& Value)
{
    Insertion(Value, 0, Value.Size() - 1);
}

}
}
}

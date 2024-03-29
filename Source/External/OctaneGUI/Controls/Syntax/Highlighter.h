/**

MIT License

Copyright (c) 2022-2024 Mitchell Davis <mdavisprog@gmail.com>

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

#include "Rules.h"

#include <memory>
#include <string>
#include <vector>

namespace OctaneGUI
{

class Text;
class TextInput;
struct TextSpan;

namespace Syntax
{

class Highlighter
{
public:
    Highlighter(TextInput& Input);
    virtual ~Highlighter();

    Highlighter& SetSymbols(const std::vector<std::u32string>& Symbols);
    const std::vector<std::u32string>& Symbols() const;

    Highlighter& SetRules(Rules& Rules_);

    Color DefaultColor() const;

    Highlighter& SetKeywordColor(Color KeywordColor);
    Color KeywordColor() const;

    std::vector<TextSpan> GetSpans(const std::u32string_view& View) const;

private:
    bool ShouldHighlight() const;

    TextInput& m_Input;
    std::vector<std::u32string> m_Symbols {};
    Rules& m_Rules { Rules::Get(U"") };
    Color m_KeywordColor { 189, 99, 197, 255 };
    Color m_SymbolColor { 86, 156, 214, 255 };
};

}
}

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

#include "../LanguageServer.h"
#include "TextInput.h"

#include <unordered_map>

namespace OctaneGUI
{

class Timer;

class TextEditor : public TextInput
{
    CLASS(TextEditor)

public:
    TextEditor(Window* InWindow);
    virtual ~TextEditor();

    TextEditor& SetMatchIndent(bool MatchIndent);
    bool MatchIndent() const;

    TextEditor& SetLineColor(const size_t Line, const Color& _Color);
    TextEditor& ClearLineColor(const size_t Line);
    TextEditor& ClearLineColors();

    TextEditor& RegisterLanguageServer();
    TextEditor& OpenFile(const char32_t* FileName);
    TextEditor& CloseFile();

    virtual void OnLoad(const Json& Root) override;

protected:
    virtual void TextAdded(const std::u32string& Contents) override;

private:
    enum class State
    {
        None,
        Connecting,
        OpenDocument,
        DocumentSymbols,
    };

    std::u32string ModifyText(const std::u32string& Pending);
    std::u32string MatchIndent(const std::u32string& Pending) const;
    std::u32string MatchCharacter(const std::u32string& Pending, char32_t Character) const;
    std::u32string ConvertTabs(const std::u32string& Pending) const;
    void PaintLineColors(Paint& Brush) const;
    bool InsertSpaces() const;

    const LanguageServer& LS() const;
    LanguageServer& LS();

    void OpenDocument();
    void RetrieveSymbols();

    bool m_MatchIndent { true };
    std::unordered_map<size_t, Color> m_LineColors {};
    std::u32string m_FileName {};
    State m_State { State::None };
    LanguageServer::ListenerID m_ListenerID { LanguageServer::INVALID_LISTENER_ID };
};

}

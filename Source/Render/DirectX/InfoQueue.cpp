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

#include "InfoQueue.hpp"
#include "../../Core/Console.hpp"

#include <d3d12.h>

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

bool InfoQueue::Initialize(ID3D12Device* Device)
{
    return Instance().InternalInitialize(Device);
}

void InfoQueue::Poll()
{
    Instance().InternalPoll();
}

InfoQueue& InfoQueue::Instance()
{
    static InfoQueue Instance;
    return Instance;
}

InfoQueue::InfoQueue()
{
}

bool InfoQueue::InternalInitialize(ID3D12Device* Device)
{
    if (Device == nullptr)
    {
        return false;
    }

    if (Device->QueryInterface(IID_PPV_ARGS(&m_InfoQueue)) != S_OK)
    {
        Core::Console::Warning("Failed to retrieve ID3D12InfoQueue interface.");
        return false;
    }

    return true;
}

void InfoQueue::InternalPoll()
{
    if (m_InfoQueue == nullptr)
    {
        return;
    }

    const UINT64 Count { m_InfoQueue->GetNumStoredMessagesAllowedByRetrievalFilter() };
    for (UINT64 I = 0; I < Count; I++)
    {
        SIZE_T Length { 0 };
        if (m_InfoQueue->GetMessage(I, nullptr, &Length) != S_FALSE)
        {
            continue;
        }

        if (Length == 0)
        {
            continue;
        }

        D3D12_MESSAGE* Message = static_cast<D3D12_MESSAGE*>(malloc(Length));
        if (m_InfoQueue->GetMessage(I, Message, &Length) == S_OK)
        {
            Core::Console::WriteLine("%s", Message->pDescription);
        }
        free(Message);
    }

    m_InfoQueue->ClearStoredMessages();
}

}
}
}

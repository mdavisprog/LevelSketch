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

#include "SwapChain.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Math/Vector2.hpp"
#include "../../Platform/Window.hpp"
#include "../../Platform/Windows/Errors.hpp"
#include "Adapter.hpp"
#include "CommandQueue.hpp"
#include "Device.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

SwapChain::SwapChain()
{
}

bool SwapChain::Initialize(Platform::Window* Window, Device const* Device_, i32 BufferCount)
{
    const Core::Math::Vector2i Size { Window->Size() };
    DXGI_SWAP_CHAIN_DESC1 SwapChainDescription { 0 };
    SwapChainDescription.BufferCount = BufferCount;
    SwapChainDescription.Width = (UINT)Size.X;
    SwapChainDescription.Height = (UINT)Size.Y;
    SwapChainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    SwapChainDescription.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain;
    const HWND Handle { reinterpret_cast<HWND>(Window->Handle()) };
    HRESULT Result { Device_->GetAdapter()->GetFactory()->CreateSwapChainForHwnd(Device_->GetCommandQueue()->Get(),
        Handle,
        &SwapChainDescription,
        nullptr,
        nullptr,
        &SwapChain) };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to create swap chain. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    Result = Device_->GetAdapter()->GetFactory()->MakeWindowAssociation(Handle, DXGI_MWA_NO_ALT_ENTER);

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to associate window handle. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    Result = SwapChain.As(&m_SwapChain);

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to cast swap chain interface. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    m_Window = Window;
    return true;
}

bool SwapChain::Present(u32 SyncInterval, u32 Flags) const
{
    HRESULT Result { m_SwapChain->Present(SyncInterval, Flags) };

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to present. Error: %s", Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    return true;
}

u32 SwapChain::BackBufferIndex() const
{
    return m_SwapChain->GetCurrentBackBufferIndex();
}

IDXGISwapChain4* SwapChain::Get() const
{
    return m_SwapChain.Get();
}

Platform::Window* SwapChain::GetWindow() const
{
    return m_Window;
}

}
}
}

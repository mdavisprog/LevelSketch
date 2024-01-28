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

#include "Adapter.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/Windows/Errors.hpp"
#include <dxgi1_6.h>

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

const auto& ErrorString = Platform::Windows::Errors::ToString;

Adapter::Adapter()
{
}

bool Adapter::Initialize()
{
    u32 FactoryFlags { 0 };

#if defined(DEBUG)
    Microsoft::WRL::ComPtr<ID3D12Debug> DebugController;
    if (D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController)) == S_OK)
    {
        DebugController->EnableDebugLayer();
        FactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    HRESULT Result { CreateDXGIFactory2(FactoryFlags, IID_PPV_ARGS(&m_Factory)) };
    if (FAILED(Result))
    {
        Core::Console::Error("Failed to create DXGIFactor. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory7> QueryFactory { nullptr };
    Result = m_Factory->QueryInterface(IID_PPV_ARGS(&QueryFactory));
    if (FAILED(Result))
    {
        Core::Console::Error("Failed to query factory interface. Error: %s", ErrorString(Result).Data());
        return false;
    }

    const auto EnumAdapter =
        [](UINT Index, IDXGIFactory7* Factory, Microsoft::WRL::ComPtr<IDXGIAdapter1>& Adapter) -> bool
    {
        return SUCCEEDED(
            Factory->EnumAdapterByGpuPreference(Index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&Adapter)));
    };

    const auto IsValidAdapter = [](IDXGIAdapter1* Adapter) -> bool
    {
        DXGI_ADAPTER_DESC1 Description {};
        HRESULT Result = Adapter->GetDesc1(&Description);

        if (FAILED(Result))
        {
            Core::Console::Error("Failed to retrieve DXGI_ADAPTER_DESC3. Error: %s", ErrorString(Result).Data());
            return false;
        }

        if (Description.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
        {
            return false;
        }

        Result = D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device9), nullptr);
        if (FAILED(Result))
        {
            return false;
        }

        return true;
    };

    Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter { nullptr };
    for (UINT I = 0; EnumAdapter(I, QueryFactory.Get(), Adapter); I++)
    {
        if (IsValidAdapter(Adapter.Get()))
        {
            break;
        }
    }

    if (Adapter.Get() == nullptr)
    {
        for (UINT I = 0; SUCCEEDED(m_Factory->EnumAdapters1(I, &Adapter)); I++)
        {
            if (IsValidAdapter(Adapter.Get()))
            {
                break;
            }
        }
    }

    m_Adapter = Adapter;

    return IsInitialized();
}

bool Adapter::IsInitialized() const
{
    return m_Adapter.Get() != nullptr;
}

IDXGIFactory7* Adapter::GetFactory() const
{
    return m_Factory.Get();
}

IDXGIAdapter1* Adapter::GetAdapter() const
{
    return m_Adapter.Get();
}

const Adapter& Adapter::FillSummary(Renderer::DriverSummary& Summary) const
{
    if (!IsInitialized())
    {
        return *this;
    }

    DXGI_ADAPTER_DESC1 Description {};
    m_Adapter->GetDesc1(&Description);
    Summary.Vendor = Core::Containers::ToString(Description.Description);

    return *this;
}

}
}
}

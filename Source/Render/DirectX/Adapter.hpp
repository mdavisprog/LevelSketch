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

#include "../Renderer.hpp"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace LevelSketch
{
namespace Render
{

namespace DirectX
{

class Adapter
{
public:
    Adapter();

    bool Initialize();
    bool IsInitialized() const;

    IDXGIFactory7* GetFactory() const;
    IDXGIAdapter1* GetAdapter() const;

    const Adapter& FillSummary(Renderer::DriverSummary& Summary) const;

private:
    Microsoft::WRL::ComPtr<IDXGIFactory7> m_Factory { nullptr };
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_Adapter { nullptr };
};

}
}
}

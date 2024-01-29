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

#include "RootSignature.hpp"
#include "../../Core/Console.hpp"
#include "../../Platform/Windows/Errors.hpp"
#include "Device.hpp"

namespace LevelSketch
{
namespace Render
{
namespace DirectX
{

static D3D12_DESCRIPTOR_RANGE1 MakeDescRange(D3D12_DESCRIPTOR_RANGE_TYPE Type, D3D12_DESCRIPTOR_RANGE_FLAGS Flags)
{
    D3D12_DESCRIPTOR_RANGE1 Result;
    Result.RangeType = Type;
    Result.NumDescriptors = 1;
    Result.BaseShaderRegister = 0;
    Result.RegisterSpace = 0;
    Result.Flags = Flags;
    Result.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    return Result;
}

RootSignature::RootSignature()
{
}

bool RootSignature::Initialize(Device const* Device_)
{
    D3D12_FEATURE_DATA_ROOT_SIGNATURE Feature { D3D_ROOT_SIGNATURE_VERSION_1_1 };
    HRESULT Result { Device_->Get()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &Feature, sizeof(Feature)) };
    if (FAILED(Result))
    {
        Feature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    D3D12_DESCRIPTOR_RANGE1 Ranges[2];
    Ranges[0] = MakeDescRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    Ranges[1] = MakeDescRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    D3D12_ROOT_PARAMETER1 Parameters[2];
    Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    Parameters[0].DescriptorTable.NumDescriptorRanges = 1;
    Parameters[0].DescriptorTable.pDescriptorRanges = &Ranges[0];

    Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    Parameters[1].DescriptorTable.NumDescriptorRanges = 1;
    Parameters[1].DescriptorTable.pDescriptorRanges = &Ranges[1];

    D3D12_STATIC_SAMPLER_DESC Sampler = { 0 };
    Sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    Sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    Sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    Sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    Sampler.MipLODBias = 0.0f;
    Sampler.MaxAnisotropy = 0;
    Sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    Sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    Sampler.MinLOD = 0.0f;
    Sampler.MaxLOD = D3D12_FLOAT32_MAX;
    Sampler.ShaderRegister = 0;
    Sampler.RegisterSpace = 0;
    Sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc { 0 };
    RootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    RootSignatureDesc.Desc_1_1.NumParameters = _countof(Parameters);
    RootSignatureDesc.Desc_1_1.pParameters = Parameters;
    RootSignatureDesc.Desc_1_1.NumStaticSamplers = 1;
    RootSignatureDesc.Desc_1_1.pStaticSamplers = &Sampler;
    RootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> Signature;
    Microsoft::WRL::ComPtr<ID3DBlob> Error;
    if (D3D12SerializeVersionedRootSignature(&RootSignatureDesc, &Signature, &Error) != S_OK)
    {
        Core::Console::Error("Failed to serialize versioned root signature. Error: %s",
            reinterpret_cast<LPCSTR>(Error->GetBufferPointer()));
        return false;
    }

    Result = Device_->Get()->CreateRootSignature(0,
        Signature->GetBufferPointer(),
        Signature->GetBufferSize(),
        IID_PPV_ARGS(&m_Signature));

    if (FAILED(Result))
    {
        Core::Console::Error("Failed to create root signature. Error: %s",
            Platform::Windows::Errors::ToString(Result).Data());
        return false;
    }

    return true;
}

ID3D12RootSignature* RootSignature::Get() const
{
    return m_Signature.Get();
}

}
}
}
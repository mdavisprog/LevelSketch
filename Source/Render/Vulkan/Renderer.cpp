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

#include "Renderer.hpp"
#include "../../Core/Console.hpp"
#include "../../Core/Math/Matrix.hpp"
#include "../../Core/Math/Rect.hpp"
#include "../../Core/Math/Vertex.hpp"
#include "../../Core/Version.hpp"
#include "../../Platform/FileSystem.hpp"
#include "../../Platform/Window.hpp"
#include "../VertexBufferDescription.hpp"
#include "../VertexDataDescription.hpp"
#include "../ViewportRect.hpp"
#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "DescriptorPool.hpp"
#include "Device.hpp"
#include "Errors.hpp"
#include "GraphicsPipeline.hpp"
#include "Loader.hpp"
#include "SwapChain.hpp"
#include "Sync.hpp"
#include "Texture.hpp"
#include "TexturePool.hpp"
#include "UniformBuffer.hpp"
#include "VertexBuffer.hpp"
#include "Viewport.hpp"

#if defined(DEBUG)
#include "DebugUtils.hpp"
#endif

namespace LevelSketch
{
namespace Render
{

UniquePtr<Renderer> Renderer::CreateInstance()
{
    return UniquePtr<Vulkan::Renderer>::New();
}

String Renderer::ShadersDirectory()
{
    return Platform::FileSystem::CombinePaths(Platform::FileSystem::ShadersDirectory(), "GLSL");
}

namespace Vulkan
{

// FIXME: Better orginazation can be used here.
#if defined(DEBUG)
static const Array<const char*> ValidationLayers { "VK_LAYER_KHRONOS_validation" };
#endif

Renderer::Renderer()
    : LevelSketch::Render::Renderer()
{
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize()
{
    if (!Loader::Instance().Initialize())
    {
        return false;
    }

    const u32 Version { VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION) };
    VkApplicationInfo AppInfo {};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = APP_NAME;
    AppInfo.applicationVersion = Version;
    AppInfo.pEngineName = APP_NAME;
    AppInfo.engineVersion = Version;
    AppInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo CreateInfo {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;
    CreateInfo.enabledLayerCount = 0;

    Array<VkExtensionProperties> ExtProperties;
    if (!Loader::Instance().GetInstanceExtensionProperties(ExtProperties))
    {
        return false;
    }

    Array<const char*> ExtPtrs;
    if (!GetRequiredExtensionProperties(ExtProperties, ExtPtrs))
    {
        return false;
    }

#if defined(DEBUG)
    Array<const char*> LayerPtrs;
    if (GetExistingLayers(ValidationLayers, LayerPtrs))
    {
        Core::Console::WriteLine("Enabling validation layers");
        CreateInfo.enabledLayerCount = static_cast<u32>(LayerPtrs.Size());
        CreateInfo.ppEnabledLayerNames = LayerPtrs.Data();

        ExtPtrs.Push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkDebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo { DebugUtils::CreateInfo() };
    CreateInfo.pNext = &DebugUtilsMessengerCreateInfo;
#endif

    CreateInfo.enabledExtensionCount = static_cast<u32>(ExtPtrs.Size());
    CreateInfo.ppEnabledExtensionNames = ExtPtrs.Data();

    VkResult Result { vkCreateInstance(&CreateInfo, nullptr, &m_Instance) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::Error("Failed to create instance. Error: %s", Errors::ToString(Result));
        return false;
    }

#if defined(DEBUG)
    DebugUtils::Instance().Initialize(m_Instance);
#endif

    return true;
}

bool Renderer::Initialize(Platform::Window* Window)
{
    UniquePtr<Viewport> Viewport_ { UniquePtr<Viewport>::New() };

    if (!Viewport_->Initialize(m_Instance, Window))
    {
        return false;
    }

    m_Viewports.Push(std::move(Viewport_));

    if (m_Device == nullptr)
    {
        Array<const char*> LayerPtrs;
#if defined(DEBUG)
        GetExistingLayers(ValidationLayers, LayerPtrs);
#endif

        m_Device = UniquePtr<Device>::New();

        if (!m_Device->Initialize(m_Instance, m_Viewports[0]->GetSurface(), LayerPtrs))
        {
            return false;
        }

        m_CommandPool = UniquePtr<CommandPool>::New();

        if (!m_CommandPool->Initialize(m_Device.Get()))
        {
            return false;
        }

        if (!m_CommandPool->InitializeBuffers(m_Device.Get(), FRAMES_IN_FLIGHT))
        {
            return false;
        }

        m_DescriptorPool = UniquePtr<DescriptorPool>::New();

        if (!m_DescriptorPool->Initialize(m_Device.Get(), FRAMES_IN_FLIGHT))
        {
            return false;
        }

        for (i32 I = 0; I < FRAMES_IN_FLIGHT; I++)
        {
            UniquePtr<Sync> Sync_ { UniquePtr<Sync>::New() };

            if (!Sync_->Initialize(m_Device.Get()))
            {
                return false;
            }

            m_Syncs.Push(std::move(Sync_));

            UniquePtr<UniformBuffer> Uniform { UniquePtr<UniformBuffer>::New() };

            if (!Uniform->Initialize(m_Device.Get()))
            {
                return false;
            }

            m_Uniforms.Push(std::move(Uniform));
        }

        m_TexturePool = UniquePtr<TexturePool>::New();
        if (!m_TexturePool->Initialize(m_Device.Get(), 100))
        {
            return false;
        }

        Core::Console::WriteLine("Initialized Vulkan");
    }

    m_Viewports.Back()->InitializeSwapChain(m_Device.Get());

    return true;
}

void Renderer::Shutdown()
{
    m_Device->WaitForIdle();

    for (const UniquePtr<GraphicsPipeline>& Pipeline : m_GraphicsPipelines)
    {
        Pipeline->Shutdown(m_Device.Get());
    }
    m_GraphicsPipelines.Clear();

    for (const UniquePtr<VertexBuffer>& Buffer : m_VertexBuffers)
    {
        Buffer->Shutdown(m_Device.Get());
    }

    for (const UniquePtr<UniformBuffer>& Uniform : m_Uniforms)
    {
        Uniform->Shutdown(m_Device.Get());
    }
    m_Uniforms.Clear();

    for (const UniquePtr<Sync>& Sync_ : m_Syncs)
    {
        Sync_->Shutdown(m_Device.Get());
    }
    m_Syncs.Clear();

    m_TexturePool->Shutdown(m_Device.Get());
    m_DescriptorPool->Shutdown(m_Device.Get());
    m_CommandPool->Shutdown(m_Device.Get());

    for (const UniquePtr<Viewport>& Viewport_ : m_Viewports)
    {
        Viewport_->Shutdown(m_Instance, m_Device.Get());
    }
    m_Viewports.Clear();

    m_Device->Shutdown();

#if defined(DEBUG)
    DebugUtils::Instance().Shutdown(m_Instance);
#endif

    if (m_Instance != nullptr)
    {
        vkDestroyInstance(m_Instance, nullptr);
        m_Instance = VK_NULL_HANDLE;
    }

    Loader::Instance().Shutdown();
}

u32 Renderer::CreateTexture(const TextureDescription& Description)
{
    Texture const* Result { m_TexturePool->AllocateTexture(m_Device.Get(), m_CommandPool.Get(), Description) };

    if (Result == nullptr)
    {
        return 0;
    }

    return Result->ID();
}

bool Renderer::BindTexture(u32 ID)
{
    const VkDescriptorSet Sets[] { m_DescriptorPool->GetSet(m_FrameIndex), m_TexturePool->Set(ID) };
    CommandBuffer const* Commands { m_CommandPool->Buffer(m_FrameIndex) };
    Commands->BindDescriptorSets(m_BoundPipeline, 2, Sets);
    return true;
}

bool Renderer::BeginRender(Platform::Window* Window, const Colorf& ClearColor)
{
    m_Syncs[m_FrameIndex]->WaitForFence(m_Device.Get());

    Viewport* Viewport_ { GetViewport(Window) };

    if (Viewport_ == nullptr)
    {
        Core::Console::Error("Failed to find associated viewport for given window.");
        return false;
    }

    Viewport_->GetSwapChain()->NextFrame(m_Device.Get(), m_Syncs[m_FrameIndex].Get());

    CommandBuffer const* Commands { m_CommandPool->Buffer(m_FrameIndex) };
    Commands->Reset();
    Commands->BeginRecord(Viewport_->GetSwapChain(), ClearColor);

    const VkExtent2D Extents { Viewport_->GetSwapChain()->Extents() };
    const ViewportRect Rect { { 0.0f, 0.0f, static_cast<f32>(Extents.width), static_cast<f32>(Extents.height) },
        0.0f,
        1.0f };
    SetViewportRect(Rect);

    const Recti Scissor { 0, 0, static_cast<i32>(Extents.width), static_cast<i32>(Extents.height) };
    SetScissor(Scissor);

    UniformBuffer* Uniforms { m_Uniforms[m_FrameIndex].Get() };
    Uniforms->GetUniforms().Perspective = Core::Math::PerspectiveMatrixLH(45.0f, Window->AspectRatio(), 0.1f, 100.0f);
    Uniforms->GetUniforms().Perspective[5] *= -1.0f;
    Uniforms->GetUniforms().Orthographic = Core::Math::OrthographicMatrixLH(Rect.Bounds, -1.0f, 1.0f);
    Uniforms->UpdateBuffer();

    m_DescriptorPool->UpdateUniform(m_Device.Get(), Uniforms, m_FrameIndex);

    return true;
}

void Renderer::EndRender(Platform::Window* Window)
{
    CommandBuffer const* Commands { m_CommandPool->Buffer(m_FrameIndex) };

    Commands->EndRecord();
    Commands->Submit(m_Device.Get(), m_Syncs[m_FrameIndex].Get());

    Viewport* Viewport_ { GetViewport(Window) };
    Viewport_->GetSwapChain()->Present(m_Device.Get(), m_Syncs[m_FrameIndex].Get());

    m_FrameIndex = (m_FrameIndex + 1) % FRAMES_IN_FLIGHT;
}

void Renderer::SetViewportRect(const ViewportRect& Rect)
{
    CommandBuffer const* Commands { m_CommandPool->Buffer(m_FrameIndex) };
    Commands->SetViewport(Rect);
}

void Renderer::SetScissor(const Recti& Rect)
{
    CommandBuffer const* Commands { m_CommandPool->Buffer(m_FrameIndex) };
    Commands->SetScissor(Rect);
}

u32 Renderer::CreateGraphicsPipeline(const GraphicsPipelineDescription& Description)
{
    UniquePtr<GraphicsPipeline> Pipeline { UniquePtr<GraphicsPipeline>::New() };

    if (!Pipeline->Initialize(m_Device.Get(),
            m_DescriptorPool.Get(),
            m_TexturePool.Get(),
            m_Viewports[0]->GetSwapChain(),
            Description))
    {
        return 0;
    }

    const u32 Result { Pipeline->ID() };
    m_GraphicsPipelines.Push(std::move(Pipeline));
    return Result;
}

bool Renderer::BindGraphicsPipeline(u32 ID)
{
    GraphicsPipeline const* Pipeline { GetGraphicsPipeline(ID) };

    if (Pipeline == nullptr)
    {
        Core::Console::Warning("Failed to bind graphics pipeline with id '%d'.", ID);
        return false;
    }

    CommandBuffer const* Commands { m_CommandPool->Buffer(m_FrameIndex) };
    Commands->BindPipeline(Pipeline);

    m_BoundPipeline = const_cast<GraphicsPipeline*>(Pipeline);

    return false;
}

void Renderer::DrawIndexed(u32 IndexCount, u32 InstanceCount, u32 StartIndex, u32 BaseVertex, u32 StartInstance)
{
    CommandBuffer const* Commands { m_CommandPool->Buffer(m_FrameIndex) };
    Commands->DrawVerticesIndexed(IndexCount, InstanceCount, StartIndex, BaseVertex, StartInstance);
}

u32 Renderer::CreateVertexBuffer(const VertexBufferDescription& Description)
{
    UniquePtr<VertexBuffer> Buffer { UniquePtr<VertexBuffer>::New() };

    if (!Buffer->Initialize(m_Device.Get(), Description))
    {
        return 0;
    }

    const u32 Result { Buffer->ID() };
    m_VertexBuffers.Push(std::move(Buffer));
    return Result;
}

bool Renderer::UploadVertexData(u32 ID, const VertexDataDescription& Description)
{
    VertexBuffer const* Buffer { GetVertexBuffer(ID) };

    if (Buffer == nullptr)
    {
        return false;
    }

    if (!Buffer->GetVertexBuffer()->Upload(m_Device.Get(),
            m_CommandPool.Get(),
            Description.VertexData,
            Description.VertexDataSize))
    {
        return false;
    }

    if (!Buffer->GetIndexBuffer()->Upload(m_Device.Get(),
            m_CommandPool.Get(),
            Description.IndexData,
            Description.IndexDataSize))
    {
        return false;
    }

    return true;
}

bool Renderer::BindVertexBuffer(u32 ID)
{
    VertexBuffer const* Buffer { GetVertexBuffer(ID) };
    CommandBuffer const* Commands { m_CommandPool->Buffer(m_FrameIndex) };
    Commands->BindBuffer(Buffer);
    return false;
}

void Renderer::UpdateViewMatrix(const Matrix4f& View)
{
    m_Uniforms[m_FrameIndex]->GetUniforms().View = View;
}

bool Renderer::GetRequiredExtensionProperties(const Array<VkExtensionProperties>& Properties,
    Array<const char*>& Ptrs) const
{
    bool FoundSurface { false };

    for (const VkExtensionProperties& Property : Properties)
    {
        const String Name { Property.extensionName };
        if (Name == "VK_KHR_surface" || Name == "VK_KHR_xlib_surface" || Name == "VK_KHR_xcb_surface" ||
            Name == "VK_KHR_wayland_surface")
        {
            FoundSurface |= Name == "VK_KHR_surface";
            Ptrs.Push(Property.extensionName);
        }
    }

    if (!FoundSurface)
    {
        Core::Console::Error("Failed to load window surface extensions.");
        return false;
    }

    return true;
}

bool Renderer::GetExistingLayers(const Array<const char*> Layers, Array<const char*>& Ptrs) const
{
    Ptrs.Clear();

    u32 Count { 0 };
    VkResult Result { vkEnumerateInstanceLayerProperties(&Count, nullptr) };
    if (Result != VK_SUCCESS)
    {
        Core::Console::WriteLine("Failed to retrieve layer properties count. Error: %s", Errors::ToString(Result));
        return false;
    }

    Array<VkLayerProperties> Properties;
    Properties.Resize(Count);
    Result = vkEnumerateInstanceLayerProperties(&Count, Properties.Data());
    if (Result != VK_SUCCESS)
    {
        Core::Console::WriteLine("Failed to retrieve layer properties. Error: %s", Errors::ToString(Result));
        return false;
    }

    for (const char* Name : Layers)
    {
        for (const VkLayerProperties& Layer : Properties)
        {
            if (String(Name) == Layer.layerName)
            {
                Ptrs.Push(Name);
            }
        }
    }

    return !Ptrs.IsEmpty();
}

VertexBuffer const* Renderer::GetVertexBuffer(u32 ID) const
{
    for (const UniquePtr<VertexBuffer>& Buffer : m_VertexBuffers)
    {
        if (Buffer->ID() == ID)
        {
            return Buffer.Get();
        }
    }

    return nullptr;
}

GraphicsPipeline const* Renderer::GetGraphicsPipeline(u32 ID) const
{
    for (const UniquePtr<GraphicsPipeline>& Pipeline : m_GraphicsPipelines)
    {
        if (Pipeline->ID() == ID)
        {
            return Pipeline.Get();
        }
    }

    return nullptr;
}

Viewport* Renderer::GetViewport(Platform::Window* Window) const
{
    for (const UniquePtr<Viewport>& Viewport_ : m_Viewports)
    {
        if (Viewport_->Window() == Window)
        {
            return Viewport_.Get();
        }
    }

    return nullptr;
}

}
}
}

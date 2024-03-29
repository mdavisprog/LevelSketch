set(CMAKE_CXX_STANDARD 17)

add_library(MurmurHash3 STATIC External/MurmurHash3/MurmurHash3.cpp)
add_subdirectory(External/OctaneGUI)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

set(TARGET LevelSketch)

# List of options that can be set.
option(SHIPPING "Enables the SHIPPING configuration." OFF)
option(WITH_TESTS "Compiles the testing framework when ON." OFF)

if(NOT WIN32 AND NOT APPLE)
    set(WITH_VULKAN ON)
endif()

if(SHIPPING)
    message("Compiling with shipping configuration.")
    set(WITH_TESTS OFF CACHE BOOL "Disabling WITH_TESTS when SHIPPING configuration is enabled." FORCE)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(BUILD_TYPE_DEBUG TRUE)
endif()

# Common source files for all platforms and configurations.
set(SOURCE
    Core/Containers/String.cpp
    Core/CommandLine.cpp
    Core/Console.cpp

    Engine/Camera.cpp
    Engine/Engine.cpp
    Engine/Type.cpp
    Engine/TypeDatabase.cpp

    Engine/ECS/Component.cpp
    Engine/ECS/ComponentPool.cpp
    Engine/ECS/Entity.cpp
    Engine/ECS/World.cpp

    GUI/GUI.cpp

    Platform/Debugger.cpp
    Platform/EventQueue.cpp
    Platform/FileSystem.cpp
    Platform/Keyboard.cpp
    Platform/Mouse.cpp
    Platform/Mutex.cpp
    Platform/Platform.cpp
    Platform/Window.cpp

    Render/Renderer.cpp

    Main/Common.cpp
)

# Files needed in non-shipping builds
if(NOT SHIPPING)
    set(SOURCE
        ${SOURCE}
        Core/Assert.cpp
    )
endif()

if(WIN32)
    set(SOURCE
        ${SOURCE}
        Platform/Windows/Debugger.cpp
        Platform/Windows/Errors.cpp
        Platform/Windows/Events.cpp
        Platform/Windows/FileSystem.cpp
        Platform/Windows/Mouse.cpp
        Platform/Windows/Mutex.cpp
        Platform/Windows/Platform.cpp
        Platform/Windows/Window.cpp

        Render/DirectX/Adapter.cpp
        Render/DirectX/CommandAllocator.cpp
        Render/DirectX/CommandList.cpp
        Render/DirectX/CommandQueue.cpp
        Render/DirectX/DepthStencil.cpp
        Render/DirectX/DescriptorHeap.cpp
        Render/DirectX/Device.cpp
        Render/DirectX/GraphicsPipeline.cpp
        Render/DirectX/Renderer.cpp
        Render/DirectX/RootSignature.cpp
        Render/DirectX/Shader.cpp
        Render/DirectX/SwapChain.cpp
        Render/DirectX/Texture.cpp
        Render/DirectX/Utility.cpp
        Render/DirectX/VertexBuffer.cpp
        Render/DirectX/Viewport.cpp
    )

    if(BUILD_TYPE_DEBUG)
        set(SOURCE
            ${SOURCE}
            Render/DirectX/InfoQueue.cpp
        )
    endif()

elseif(APPLE)
    set(SOURCE
        ${SOURCE}
        Platform/Mac/DisplayLink.mm
        Platform/Mac/EventView.mm
        Platform/Mac/FileSystem.mm
        Platform/Mac/Mouse.mm
        Platform/Mac/Platform.mm
        Platform/Mac/Window.mm
        Platform/Mac/WindowBridge.mm

        Render/Metal/CommandBuffer.mm
        Render/Metal/CommandEncoder.mm
        Render/Metal/CommandQueue.mm
        Render/Metal/DepthStencil.mm
        Render/Metal/Device.mm
        Render/Metal/GraphicsPipeline.mm
        Render/Metal/Renderer.mm
        Render/Metal/Shader.mm
        Render/Metal/Texture.mm
        Render/Metal/VertexBuffer.mm
        Render/Metal/View.mm
        Render/Metal/ViewController.mm
    )
else()
    set(SOURCE
        ${SOURCE}
        Platform/SDL2/FileSystem.cpp
        Platform/SDL2/Mouse.cpp
        Platform/SDL2/Platform.cpp
        Platform/SDL2/Window.cpp
    )

    set(SOURCE
        ${SOURCE}
        Render/Vulkan/Buffer.cpp
        Render/Vulkan/CommandBuffer.cpp
        Render/Vulkan/CommandPool.cpp
        Render/Vulkan/DescriptorPool.cpp
        Render/Vulkan/Device.cpp
        Render/Vulkan/GraphicsPipeline.cpp
        Render/Vulkan/Loader.cpp
        Render/Vulkan/LogicalDevice.cpp
        Render/Vulkan/PhysicalDevice.cpp
        Render/Vulkan/Queue.cpp
        Render/Vulkan/Renderer.cpp
        Render/Vulkan/Sampler.cpp
        Render/Vulkan/Shader.cpp
        Render/Vulkan/Surface.cpp
        Render/Vulkan/SurfaceX11.cpp
        Render/Vulkan/SwapChain.cpp
        Render/Vulkan/Sync.cpp
        Render/Vulkan/Texture.cpp
        Render/Vulkan/TexturePool.cpp
        Render/Vulkan/UniformBuffer.cpp
        Render/Vulkan/VertexBuffer.cpp
        Render/Vulkan/Viewport.cpp
    )

    if(BUILD_TYPE_DEBUG)
        set(SOURCE
            ${SOURCE}
            Render/Vulkan/DebugUtils.cpp
        )
    endif()
endif()

# The main function.
if(APPLE)
    set(SOURCE ${SOURCE} Main/Main.mm)
else()
    set(SOURCE ${SOURCE} Main/Main.cpp)
endif()

# Testing framework
if(WITH_TESTS)
    set(SOURCE
        ${SOURCE}

        Tests/Core/Array.cpp
        Tests/Core/CommandLine.cpp
        Tests/Core/HashMap.cpp
        Tests/Core/HashSet.cpp
        Tests/Core/List.cpp
        Tests/Core/Map.cpp
        Tests/Core/Math.cpp
        Tests/Core/Optional.cpp
        Tests/Core/RedBlackTree.cpp
        Tests/Core/Shareable.cpp
        Tests/Core/SharedPtr.cpp
        Tests/Core/Sort.cpp
        Tests/Core/String.cpp
        Tests/Core/UniquePtr.cpp
        Tests/Core/WeakPtr.cpp

        Tests/Engine/Class.cpp
        Tests/Engine/ECS.cpp
        Tests/Engine/TypeDatabase.cpp

        Tests/Platform/FileSystem.cpp

        Tests/System.cpp
        Tests/TestSuite.cpp
    )
endif()

if(APPLE)
    add_executable(
        ${TARGET}
        MACOSX_BUNDLE
        ${SOURCE}
    )
    
    # Enable ARC
    set_property(
        TARGET ${TARGET}
        APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc"
    )

    set_target_properties(
        ${TARGET}
        PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER my.domain.style.identifier.${TARGET}
        MACOSX_BUNDLE_BUNDLE_NAME ${TARGET}
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Platform/Mac/Info.plist
    )
else()
    add_executable(
        ${TARGET}
        ${SOURCE}
    )
endif()

#
# Link Libraries
#

set(LIBS MurmurHash3 OctaneGUI)

if(WIN32)
    set(LIBS
        ${LIBS}
        d3d12
        d3dcompiler
        dxgi
    )
elseif(APPLE)
    set(LIBS
        ${LIBS}
        "-framework AppKit"
        "-framework CoreVideo"
        "-framework Foundation"
        "-framework Metal"
        "-framework MetalKit"
        "-framework QuartzCore"
    )
else()
    find_package(SDL2 REQUIRED)

    set(LIBS
        ${LIBS}
        ${SDL2_LIBRARIES}
    )

    target_include_directories(
        ${TARGET}
        PRIVATE ${SDL2_INCLUDE_DIRS}
    )

    if(VULKAN_SDK)
        set(VULKAN_INCLUDE ${VULKAN_SDK}/include)
        set(VULKAN_LIBRARIES ${VULKAN_SDK}/lib/libvulkan.so.1)
    else()
        message(FATAL_ERROR "VULKAN_SDK is not defined! Please define this as an environment variable within the build system.")
    endif()

    target_include_directories(
        ${TARGET}
        PRIVATE ${VULKAN_INCLUDE}
    )

    set(LIBS
        ${LIBS}
        ${VULKAN_LIBRARIES}
    )
endif()

target_link_libraries(
    ${TARGET}
    ${LIBS}
)

#
# Compile Definitions
#

set(DEFINES "")

if(BUILD_TYPE_DEBUG)
    set(DEFINES
        ${DEFINES}
        DEBUG
    )
endif()

if(WIN32)
    set(DEFINES
        ${DEFINES}
        PLATFORM_WINDOWS
        RENDER_DIRECTX
    )
elseif(APPLE)
    set(DEFINES
        ${DEFINES}
        PLATFORM_MAC
        RENDER_METAL
    )
else()
    set(DEFINES
        ${DEFINES}
        PLATFORM_SDL2
    )

    set(DEFINES
        ${DEFINES}
        RENDER_VULKAN
    )
endif()

if(WITH_TESTS)
    set(DEFINES
        ${DEFINES}
        WITH_TESTS
    )
endif()

if(SHIPPING)
    set(DEFINES
        ${DEFINES}
        SHIPPING
    )
endif()

target_compile_definitions(
    ${TARGET}
    PRIVATE ${DEFINES}
)

if(APPLE)
    message("Resources are copied into the binary directory for now.")
endif()

add_custom_command(
    TARGET ${TARGET}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/../Content"
    "$<TARGET_FILE_DIR:${TARGET}>/Content"
)

#
# Compile Shaders
#

if(WITH_VULKAN)
    set(SHADER_FILES
        ${CMAKE_CURRENT_LIST_DIR}/../Content/Shaders/GLSL/GUIPS.frag
        ${CMAKE_CURRENT_LIST_DIR}/../Content/Shaders/GLSL/GUIVS.vert
        ${CMAKE_CURRENT_LIST_DIR}/../Content/Shaders/GLSL/TestPS.frag
        ${CMAKE_CURRENT_LIST_DIR}/../Content/Shaders/GLSL/TestVS.vert
    )

    foreach(SHADER_FILE ${SHADER_FILES})
        get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
        set(OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/Content/Shaders/GLSL/${SHADER_NAME})

        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND "${VULKAN_SDK}/bin/glslc" "${SHADER_FILE}" -o "${OUTPUT_FILE}"
            DEPENDS ${TARGET}
        )

        add_custom_target(
            COMPILE_${SHADER_NAME}
            ALL
            DEPENDS ${OUTPUT_FILE}
        )
    endforeach()
endif()

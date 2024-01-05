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

if(SHIPPING)
    message("Compiling with shipping configuration.")
    set(WITH_TESTS OFF CACHE BOOL "Disabling WITH_TESTS when SHIPPING configuration is enabled." FORCE)
endif()

# Common source files for all platforms and configurations.
set(SOURCE
    Core/Containers/String.cpp
    Core/CommandLine.cpp
    Core/Console.cpp

    Engine/Engine.cpp
    Engine/Type.cpp
    Engine/TypeDatabase.cpp

    Platform/Debugger.cpp
    Platform/EventQueue.cpp
    Platform/FileSystem.cpp
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
        Platform/Windows/Events.cpp
        Platform/Windows/FileSystem.cpp
        Platform/Windows/Mutex.cpp
        Platform/Windows/Platform.cpp
        Platform/Windows/Window.cpp

        Render/DirectX/RenderBuffer.cpp
        Render/DirectX/Renderer.cpp
        Render/DirectX/Shader.cpp
        Render/DirectX/Texture.cpp
        Render/DirectX/Utility.cpp
    )

    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(SOURCE
            ${SOURCE}
            Render/DirectX/InfoQueue.cpp
        )
    endif()

elseif(APPLE)
    set(SOURCE
        ${SOURCE}
        Platform/Mac/FileSystem.mm
        Platform/Mac/Platform.mm
        Platform/Mac/Window.mm

        Render/Metal/Renderer.mm
    )
else()
    set(SOURCE
        ${SOURCE}
        Platform/SDL2/FileSystem.cpp
        Platform/SDL2/Platform.cpp
        Platform/SDL2/Window.cpp

        Render/OpenGL/Extensions.cpp
        Render/OpenGL/Renderer.cpp
        Render/OpenGL/SDL2Renderer.cpp
    )
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
        Tests/Core/List.cpp
        Tests/Core/Map.cpp
        Tests/Core/Math.cpp
        Tests/Core/RedBlackTree.cpp
        Tests/Core/Shareable.cpp
        Tests/Core/SharedPtr.cpp
        Tests/Core/String.cpp
        Tests/Core/UniquePtr.cpp
        Tests/Core/WeakPtr.cpp

        Tests/Engine/Class.cpp
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
        "-framework Foundation"
        "-framework Metal"
        "-framework MetalKit"
    )
else()
    find_package(SDL2 REQUIRED)
    find_package(OpenGL REQUIRED)

    set(LIBS
        ${LIBS}
        ${SDL2_LIBRARIES}
        ${OPENGL_LIBRARIES}
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

if(CMAKE_BUILD_TYPE MATCHES Debug)
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
        RENDER_OPENGL
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

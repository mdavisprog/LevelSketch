set(CMAKE_CXX_STANDARD 17)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

set(TARGET LevelSketch)

# On Linux, SDL2 is used for the platform.
if(CMAKE_SYSTEM_NAME MATCHES Linux)
    find_package(SDL2 REQUIRED)
    find_package(OpenGL REQUIRED)
endif()

add_subdirectory(External/OctaneGUI)

# Common source files for all platforms and configurations.
set(SOURCE
    Platform/Platform.cpp

    Render/Renderer.cpp

    Main/Common.cpp
)

if(WIN32)
    set(SOURCE
        ${SOURCE}
        Platform/Windows/Events.cpp
        Platform/Windows/Platform.cpp
        Platform/Windows/Window.cpp
    
        Render/DirectX/Renderer.cpp
    )
elseif(APPLE)
    set(SOURCE
        ${SOURCE}
        Platform/Mac/Platform.mm
        Platform/Mac/Window.mm

        Render/Metal/Renderer.mm
    )
else()
    set(SOURCE
        ${SOURCE}
        Platform/SDL2/Platform.cpp
        Platform/SDL2/Window.cpp

        Render/OpenGL/Renderer.cpp
    )
endif()

# The main function.
if(APPLE)
    set(SOURCE ${SOURCE} Main/Main.mm)
else()
    set(SOURCE ${SOURCE} Main/Main.cpp)
endif()

if(WITH_TESTS)
    set(SOURCE
        ${SOURCE}

        Tests/Core/Array.cpp
        Tests/Core/Shareable.cpp
        Tests/Core/SharedPtr.cpp
        Tests/Core/String.cpp
        Tests/Core/UniquePtr.cpp
        Tests/Core/WeakPtr.cpp

        Tests/System.cpp
        Tests/TestSuite.cpp
    )
endif()

add_executable(
    ${TARGET}
    ${SOURCE}
)

#
# Link Libraries
#

set(LIBS OctaneGUI)

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
    )
else()
    set(LIBS
        ${LIBS}
        ${SDL2_LIBRARIES}
        ${OpenGL_LIBS}
    )
endif()

target_link_libraries(
    ${TARGET}
    ${LIBS}
)

if(WITH_TESTS)
    target_compile_definitions(
        ${TARGET}
        PRIVATE WITH_TESTS
    )
endif()

if(SHIPPING)
    target_compile_definitions(
        ${TARGET}
        PRIVATE SHIPPING
    )
endif()

if(APPLE)
    file(COPY Platform/Mac/Info.plist DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY Platform/Mac/MainMenu.storyboard DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

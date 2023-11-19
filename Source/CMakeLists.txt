set(CMAKE_CXX_STANDARD 17)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

set(TARGET LevelSketch)

add_subdirectory(External/OctaneGUI)

set(SOURCE
    Render/Renderer.cpp

    Main.cpp
)

if(WIN32)
    set(SOURCE
        ${SOURCE}
        Platform/Windows/Events.cpp
        Platform/Windows/Platform.cpp
        Platform/Windows/Window.cpp
    
        Render/DirectX/Renderer.cpp
    )
endif()

if(WITH_TESTS)
    set(SOURCE
        ${SOURCE}

        Tests/Core/Array.cpp

        Tests/System.cpp
        Tests/TestSuite.cpp
    )
endif()

add_executable(
    ${TARGET}
    ${SOURCE}
)

target_include_directories(
    ${TARGET}
    PRIVATE External
)

target_link_libraries(
    ${TARGET}
    OctaneGUI
)

if(WIN32)
    target_link_libraries(
        ${TARGET}
        d3d12
        d3dcompiler
        dxgi
    )
endif()

if(WITH_TESTS)
    target_compile_definitions(
        ${TARGET}
        PRIVATE WITH_TESTS
    )
endif()

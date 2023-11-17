set(CMAKE_CXX_STANDARD 17)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

set(TARGET LevelSketch)

add_subdirectory(External/OctaneGUI)

set(SOURCE Main.cpp)

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
    d3d12
    d3dcompiler
    dxgi
)
